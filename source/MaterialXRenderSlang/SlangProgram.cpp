//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#if defined(_WIN32)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
#endif

#include <MaterialXRenderSlang/SlangProgram.h>
#include <MaterialXRenderSlang/SlangTextureHandler.h>
#include <MaterialXRenderSlang/SlangTypeUtils.h>
#include <MaterialXRenderSlang/SlangFramebuffer.h>
#include <MaterialXGenHw/HwShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <iostream>

MATERIALX_NAMESPACE_BEGIN

namespace
{

const float PI = std::acos(-1.0f);

} // anonymous namespace

SlangProgramPtr SlangProgram::create(SlangContextPtr context)
{
    return SlangProgramPtr(new SlangProgram(std::move(context)));
}

SlangProgram::SlangProgram(SlangContextPtr context) :
    _context(std::move(context))
{
}

SlangProgram::~SlangProgram()
{
    clearBuiltData();
}

void SlangProgram::setStages(ShaderPtr shader)
{
    if (!shader)
    {
        throw ExceptionRenderError("Cannot set stages using null hardware shader");
    }

    _shader = shader;
    for (size_t i = 0; i < shader->numStages(); ++i)
    {
        const ShaderStage& stage = shader->getStage(i);
        addStage(stage.getName(), stage.getSourceCode());
    }

    _name = _shader->getName();
}

void SlangProgram::addStage(const string& stage, const string& sourceCode)
{
    _stages[stage] = sourceCode;
}

const string& SlangProgram::getStageSourceCode(const string& stage) const
{
    auto it = _stages.find(stage);
    if (it != _stages.end())
    {
        return it->second;
    }
    return EMPTY_STRING;
}

void SlangProgram::clearStages()
{
    _stages.clear();
    clearBuiltData();
}

bool SlangProgram::build()
{
    using namespace rhi;

    clearBuiltData();

    ComPtr<slang::ISession> slangSession = _context->getDevice()->getSlangSession();

    ComPtr<slang::IBlob> diagnosticsBlob;
    std::vector<slang::IComponentType*> componentTypes;
    StringVec diagnosticVec;

    _name = _context->deduplicateName(_name);

    const std::string& vertexShaderSource = _stages[Stage::VERTEX];
    if (!vertexShaderSource.empty())
    {
        slang::IModule* module = slangSession->loadModuleFromSourceString(
            (_name + "VertexShaders").c_str(),
            (_name + ".VertexShaders").c_str(),
            vertexShaderSource.c_str(),
            diagnosticsBlob.writeRef());

        if (diagnosticsBlob)
            diagnosticVec.push_back((const char*) diagnosticsBlob->getBufferPointer());

        if (!module)
            throw ExceptionRenderError("Failed to compile vertex shaders", diagnosticVec);

        ComPtr<slang::IEntryPoint> entryPoint;
        SLANG_RETURN_ON_FAIL(module->findEntryPointByName("vertexMain", entryPoint.writeRef()));

        componentTypes.push_back(module);
        componentTypes.push_back(entryPoint);
    }

    const std::string& pixelShaderSource = _stages[Stage::PIXEL];
    if (!pixelShaderSource.empty())
    {
        slang::IModule* module = slangSession->loadModuleFromSourceString(
            (_name + "PixelShaders").c_str(),
            (_name + ".PixelShaders").c_str(),
            pixelShaderSource.c_str(),
            diagnosticsBlob.writeRef());

        if (diagnosticsBlob)
            diagnosticVec.push_back((const char*) diagnosticsBlob->getBufferPointer());

        if (!module)
            throw ExceptionRenderError("Failed to compile frament shaders", diagnosticVec);

        ComPtr<slang::IEntryPoint> entryPoint;
        SLANG_RETURN_ON_FAIL(module->findEntryPointByName("fragmentMain", entryPoint.writeRef()));

        componentTypes.push_back(module);
        componentTypes.push_back(entryPoint);
    }

    ComPtr<slang::IComponentType> composedProgram;
    Result result = slangSession->createCompositeComponentType(
        componentTypes.data(),
        componentTypes.size(),
        composedProgram.writeRef(),
        diagnosticsBlob.writeRef());
    SLANG_RETURN_ON_FAIL(result);

    ComPtr<slang::IComponentType> linkedProgram;
    result = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
    if (SLANG_FAILED(result))
    {
        if (diagnosticsBlob)
            diagnosticVec.push_back((const char*) diagnosticsBlob->getBufferPointer());
        throw ExceptionRenderError("Failed to link program", diagnosticVec);
    }

    _slangReflection = linkedProgram->getLayout();
    _shaderProgram = _context->getDevice()->createShaderProgram(linkedProgram, diagnosticsBlob.writeRef());

    if (!_shaderProgram)
    {
        if (diagnosticsBlob)
            diagnosticVec.push_back((const char*) diagnosticsBlob->getBufferPointer());
        throw ExceptionRenderError("Failed to create shader program", diagnosticVec);
    }

    _rootObject = _context->getDevice()->createRootShaderObject(_shaderProgram.get());
    if (!_rootObject)
        return false;

    updateVertexInputList();
    updateUniformsList();
    bindUniformDefaults();

    _alphaBlendingEnabled = _shader && _shader->hasAttribute(HW::ATTR_TRANSPARENT);

    if (isTransparent())
    {
        getPipeline(SlangProgram::PipelineKind2::CullFront, rhi::Format::RGBA8UnormSrgb);
        getPipeline(SlangProgram::PipelineKind2::Default, rhi::Format::RGBA8UnormSrgb);
    }
    else
    {
        getPipeline(SlangProgram::PipelineKind2::Default, rhi::Format::RGBA8UnormSrgb);
    }

    return true;
}

rhi::ComPtr<rhi::IRenderPipeline> SlangProgram::getPipeline(PipelineKind2 kind, rhi::Format colorFormat)
{

    PipelineKey key{ kind, colorFormat };

    auto it = _pipelines.find(key);

    if (it != _pipelines.end())
    {
        return it->second;
    }

    if (!_shaderProgram)
    {
        throw ExceptionRenderError("Cannot build pipeline without an existing shader program.");
    }

    if (is_set(kind, PipelineKind2::CullBack) && is_set(kind, PipelineKind2::CullFront))
    {
        throw ExceptionRenderError("Cull can be only either front or back, not both.");
    }

    rhi::ColorTargetDesc colorTarget = {};
    colorTarget.format = colorFormat;
    if (is_set(kind, PipelineKind2::AlphaOn) || (isTransparent() && !is_set(kind, PipelineKind2::AlphaOff)))
    {
        colorTarget.color.srcFactor = rhi::BlendFactor::SrcAlpha;
        colorTarget.color.dstFactor = rhi::BlendFactor::InvSrcAlpha;
        colorTarget.alpha.srcFactor = rhi::BlendFactor::SrcAlpha;
        colorTarget.alpha.dstFactor = rhi::BlendFactor::InvSrcAlpha;
        colorTarget.enableBlend = true;
    }

    rhi::RenderPipelineDesc pipelineDesc = {};
    pipelineDesc.program = _shaderProgram;
    pipelineDesc.inputLayout = _inputLayout;
    pipelineDesc.depthStencil.format = rhi::Format::D32Float;
    pipelineDesc.depthStencil.depthTestEnable = true;
    pipelineDesc.depthStencil.depthWriteEnable = true;
    pipelineDesc.targets = &colorTarget;
    pipelineDesc.targetCount = 1;
    if (is_set(kind, PipelineKind2::Wireframe))
    {
        pipelineDesc.rasterizer.fillMode = rhi::FillMode::Wireframe;
    }
    if (is_set(kind, PipelineKind2::CullBack))
    {
        pipelineDesc.rasterizer.cullMode = rhi::CullMode::Back;
    }
    else if (is_set(kind, PipelineKind2::CullFront))
    {
        pipelineDesc.rasterizer.cullMode = rhi::CullMode::Front;
    }

    auto pipeline = _context->getDevice()->createRenderPipeline(pipelineDesc);
    if (!pipeline)
        throw ExceptionRenderError("Failed to compile pipeline.");
    _pipelines[key] = pipeline;

    return pipeline;
}

rhi::ComPtr<rhi::IRenderPipeline> SlangProgram::getPipeline(PipelineKind2 kind, const SlangFramebufferPtr& framebuffer)
{
    return getPipeline(kind, framebuffer->getColorTexture()->getDesc().format);
}

void SlangProgram::clearBuiltData()
{
    _slangReflection = nullptr;
    _uniformList.clear();
    _vertexInputsList.clear();
    _pipelines.clear();
    _rootObject = {};
    _explicitBoundImages = {};
    _inputLayout = {};
    // Must be cleared, because we might have converted the mesh data
    // to accomodate the required inputs, e.g., extending float2 to float3
    _vertexInputBuffers = {};
}

const SlangProgram::VertexInputMap& SlangProgram::getVertexInputsList()
{
    return updateVertexInputList();
}

const SlangProgram::UniformInputMap& SlangProgram::getUniformsList()
{
    return updateUniformsList();
}

bool SlangProgram::hasUniform(const string& name)
{
    const UniformInputMap& uniformList = getUniformsList();
    return uniformList.find(name) != uniformList.end();
}

void SlangProgram::bindUniform(const string& name, ConstValuePtr value, bool errorIfMissing)
{
    const UniformInputMap& uniformList = getUniformsList();
    auto input = uniformList.find(name);
    if (input == uniformList.end() || input->second->slangCursors.empty())
    {
        if (errorIfMissing)
        {
            throw ExceptionRenderError("Unknown uniform: " + name);
        }
        return;
    }

    if (value->getValueString() == EMPTY_STRING)
    {
        for (auto& cursor : input->second->slangCursors)
            clearSlangValue(cursor, input->second->slangByteSize);
    }

    for (auto& cursor : input->second->slangCursors)
        setSlangValue(cursor, value, input->second->slangByteSize);
}

rhi::ComPtr<rhi::IBuffer> SlangProgram::bindPartition(MeshPartitionPtr part)
{
    if (!part || part->getFaceCount() == 0)
    {
        throw ExceptionRenderError("Cannot bind geometry partition");
    }

    auto it = _indexBuffers.find(part);
    if (it == _indexBuffers.end())
    {
        MeshIndexBuffer& indexData = part->getIndices();
        size_t indexBufferSize = indexData.size();

        rhi::BufferDesc indexBufferDesc;
        indexBufferDesc.size = indexBufferSize * sizeof(uint32_t);
        indexBufferDesc.usage = rhi::BufferUsage::IndexBuffer;
        indexBufferDesc.defaultState = rhi::ResourceState::IndexBuffer;
        auto buffer = _context->getDevice()->createBuffer(indexBufferDesc, indexData.data());
        it = _indexBuffers.insert(std::make_pair(part, buffer)).first;
    }

    if (_renderState)
        _renderState->indexBuffer = it->second;
    return it->second;
}

void SlangProgram::drawPartition(MeshPartitionPtr part)
{
    if (!_renderState || !_passEncoder || _renderState->indexBuffer != _indexBuffers[part])
        throw ExceptionRenderError("Cannot draw unbound partition");

    rhi::DrawArguments drawArgs = {};
    drawArgs.vertexCount = (uint32_t) part->getIndices().size();
    _passEncoder->setRenderState(*_renderState);
    _passEncoder->drawIndexed(drawArgs);
}

void SlangProgram::bindMesh(MeshPtr mesh, rhi::RenderState& renderState)
{
    if (!_rootObject)
    {
        throw ExceptionRenderError("Cannot bind geometry without a valid program");
    }
    if (!mesh)
    {
        throw ExceptionRenderError("No mesh to bind");
    }

    if (mesh != _boundMesh)
    {
        unbindGeometry();
        bindUniformGeomprops(mesh);
    }

    _boundMesh = mesh;

    bindVertexInputs(mesh, renderState);
}

rhi::ComPtr<rhi::IBuffer> SlangProgram::getIndexBuffer(MeshPartitionPtr partition)
{
    if (auto it = _indexBuffers.find(partition); it != _indexBuffers.end())
        return it->second;
    return {};
}

void SlangProgram::unbindGeometry()
{
    _vertexInputBuffers.clear();
    _indexBuffers.clear();
    _boundMesh = {};
}

void SlangProgram::bindUniformDefaults()
{
    const UniformInputMap& uniformList = getUniformsList();
    for (auto& input : uniformList)
    {
        /// Textures are bound in bindTextures and bindLighting, as we need imageHandler for that.
        if (isSamplerTexture2D(input.second->slangCursors.front().m_typeLayout))
            continue;

        if (input.second->slangDefaultValue && input.second->slangDefaultValue->getTypeString() != Type::STRING.getName())
        {
            for (auto& cursor : input.second->slangCursors)
                setSlangValue(cursor, input.second->slangDefaultValue, input.second->slangByteSize);
        }
        else
        {
            for (auto& cursor : input.second->slangCursors)
                clearSlangValue(cursor, input.second->slangByteSize);
        }
    }
}

void SlangProgram::bindUniformGeomprops(MeshPtr mesh)
{
    static const std::string GEOMPROP_PREFIX = HW::GEOMPROP + "_";
    /// For now, mesh has no actual geomprops, so we just bind any int,
    /// and float1-4 geomprop information.
    /// This is matching the code in GlslProgram that sets 1 for int,
    /// 0 for float1-3, and 0,0,0,1 for float4.
    const UniformInputMap& uniformList = getUniformsList();
    for (auto& it : uniformList)
    {
        const UniformInput& input = *it.second;
        // Check if the name starts with GEOMPROP_PREFIX
        if (input.name.rfind(GEOMPROP_PREFIX, 0) != 0)
            continue;

        slang::TypeLayoutReflection* typeLayout = input.slangCursors.front().m_typeLayout;
        if (typeLayout->getKind() == slang::TypeReflection::Kind::Scalar)
        {
            if (typeLayout->getScalarType() == slang::TypeReflection::ScalarType::Int32)
            {
                bindUniform(input.name, TypedValue<int>::createValue(1));
            }
            else if (typeLayout->getScalarType() == slang::TypeReflection::ScalarType::Float32)
            {
                bindUniform(input.name, TypedValue<float>::createValue(0.f));
            }
        }
        else if (typeLayout->getKind() == slang::TypeReflection::Kind::Vector)
        {
            if (typeLayout->getElementTypeLayout()->getScalarType() == slang::TypeReflection::ScalarType::Float32)
            {
                switch (typeLayout->getColumnCount())
                {
                    case 1:
                        bindUniform(input.name, TypedValue<float>::createValue(0.f));
                    case 2:
                        bindUniform(input.name, TypedValue<Vector2>::createValue(Vector2(0.f, 0.f)));
                    case 3:
                        bindUniform(input.name, TypedValue<Vector3>::createValue(Vector3(0.f, 0.f, 0.f)));
                    case 4:
                        bindUniform(input.name, TypedValue<Vector4>::createValue(Vector4(0.f, 0.f, 0.f, 1.f)));
                    default:
                        throw ExceptionRenderError("Invalid type in toSlangFormat");
                }
            }
        }
    }
}

void SlangProgram::bindVertexInputs(MeshPtr mesh, rhi::RenderState& renderState)
{
    if (!mesh)
    {
        throw ExceptionRenderError("No geometry set to bind");
    }

    if (_vertexInputsList.empty())
    {
        throw ExceptionRenderError("No vertex inputs ready");
    }

    std::vector<uint8_t> restructuredData;
    size_t vertexCount = mesh->getVertexCount();
    for (auto viIter : _vertexInputsList)
    {
        const std::string& name = viIter.first;
        const auto& input = viIter.second;

        unsigned int index = input->valueSemanticIndex;

        unsigned int stride = 0;
        MeshStreamPtr stream = mesh->getStream(name);
        if (!stream)
        {
            throw ExceptionRenderError("Geometry buffer could not be retrieved for binding: " + name + ". Index: " + std::to_string(index));
        }
        MeshFloatBuffer& attributeData = stream->getData();
        stride = stream->getStride();

        if (attributeData.empty() || (stride == 0))
        {
            throw ExceptionRenderError("Geometry buffer could not be retrieved for binding: " + name + ". Index: " + std::to_string(index));
        }

        auto bufferIter = _vertexInputBuffers.find(name);
        if (bufferIter == _vertexInputBuffers.end())
        {
            size_t shaderByteSize = getByteSize(input->slangTypeLayout);
            size_t meshByteSize = stride * sizeof(float);
            const void* bufferData = attributeData.data();

            if (shaderByteSize != meshByteSize)
            {
                restructuredData.assign(shaderByteSize * vertexCount, 0u);
                for (size_t i = 0; i < vertexCount; ++i)
                {
                    memcpy(&restructuredData[i * shaderByteSize],
                           &attributeData[i * stride],
                           std::min(shaderByteSize, meshByteSize));
                }
                bufferData = restructuredData.data();
            }

            rhi::BufferDesc vertexBufferDesc;
            vertexBufferDesc.size = vertexCount * shaderByteSize;
            vertexBufferDesc.usage = rhi::BufferUsage::VertexBuffer;
            vertexBufferDesc.defaultState = rhi::ResourceState::VertexBuffer;
            std::tie(bufferIter, std::ignore) = _vertexInputBuffers.insert(
                std::make_pair(name,
                               _context->getDevice()->createBuffer(vertexBufferDesc, bufferData)));
        }

        renderState.vertexBuffers[input->bufferIndex] = bufferIter->second;
    }

    renderState.vertexBufferCount = (uint32_t) _vertexInputsList.size();
}

void SlangProgram::bindTextures(ImageHandlerPtr imageHandler)
{
    if (!isBuilt())
    {
        throw ExceptionRenderError("Cannot bind textures without a valid program");
    }

    SlangTextureHandler* slangTextureHandler = dynamic_cast<SlangTextureHandler*>(imageHandler.get());
    if (!slangTextureHandler)
    {
        throw ExceptionRenderError("Cannot bind textures without a Slang specific image handler");
    }

    // Bind textures based on uniforms found in the program
    const UniformInputMap& uniformList = getUniformsList();
    const VariableBlock& publicUniforms = _shader->getStage(Stage::PIXEL).getUniformBlock(HW::PUBLIC_UNIFORMS);
    for (const auto& uniform : uniformList)
    {
        if (isSamplerTexture2D(uniform.second->slangCursors.front().m_typeLayout))
        {
            // Always bind a texture unless it is a lighting texture.
            // Lighting textures are handled in the bindLighting() call.
            // If no texture can be loaded then the default color defined in
            // "samplingProperties" will be used to create a fallback texture.
            if (uniform.first != HW::ENV_RADIANCE &&
                uniform.first != HW::ENV_IRRADIANCE)
            {
                /// If the texture is already explicitly bound, we skip the default value binding.
                if (auto it = _explicitBoundImages.find(uniform.first); it == _explicitBoundImages.end())
                {
                    const string fileName(uniform.second->slangDefaultValue ? uniform.second->slangDefaultValue->getValueString() : "");

                    ImageSamplingProperties samplingProperties;
                    samplingProperties.setProperties(uniform.first, publicUniforms);

                    ImagePtr image = slangTextureHandler->acquireImage(fileName, samplingProperties.defaultColor);
                    if (slangTextureHandler->bindImage(image, samplingProperties))
                    {
                        for (auto& cursor : uniform.second->slangCursors)
                            slangTextureHandler->bindImage(cursor, image);
                    }
                }
            }
        }
    }
}

void SlangProgram::bindTexture(ImageHandlerPtr imageHandler,
                               const std::string& shaderTextureName,
                               ImagePtr imagePtr,
                               const ImageSamplingProperties& samplingProperties)
{
    SlangTextureHandler* slangTextureHandler = dynamic_cast<SlangTextureHandler*>(imageHandler.get());
    if (!slangTextureHandler)
    {
        throw ExceptionRenderError("Cannot bind textures without a Slang specific image handler");
    }

    if (imageHandler->bindImage(imagePtr, samplingProperties))
    {
        _explicitBoundImages[shaderTextureName] = imagePtr;
        const UniformInputMap& uniformList = getUniformsList();
        if (auto it = uniformList.find(shaderTextureName); it != uniformList.end())
        {
            if (!isSamplerTexture2D(it->second->slangCursors.front().m_typeLayout))
                throw ExceptionRenderError("Binding a texture to a non-texture shader variable " + shaderTextureName);

            for (auto& cursor : it->second->slangCursors)
                slangTextureHandler->bindImage(cursor, imagePtr);
        }
    }
}

void SlangProgram::bindLighting(LightHandlerPtr lightHandler, ImageHandlerPtr imageHandler)
{
    if (!lightHandler)
    {
        // Nothing to bind if a light handler is not used. This is a valid condition
        // for shaders that don't need lighting, so just exit silently.
        return;
    }

    if (!isBuilt())
    {
        throw ExceptionRenderError("Cannot bind without a valid program");
    }

    SlangTextureHandler* slangTextureHandler = dynamic_cast<SlangTextureHandler*>(imageHandler.get());
    if (!slangTextureHandler)
    {
        throw ExceptionRenderError("Cannot bind textures without a Slang specific image handler");
    }

    // Bind environment lighting properties.
    Matrix44 envRotation = Matrix44::createRotationY(PI) * lightHandler->getLightTransform().getTranspose();
    bindUniform(HW::ENV_MATRIX, Value::createValue(envRotation), false);
    bindUniform(HW::ENV_RADIANCE_SAMPLES, Value::createValue(lightHandler->getEnvSampleCount()), false);
    bindUniform(HW::ENV_LIGHT_INTENSITY, Value::createValue(lightHandler->getEnvLightIntensity()), false);
    ImagePtr envRadiance = nullptr;
    if (lightHandler->getIndirectLighting())
    {
        envRadiance = lightHandler->getUsePrefilteredMap() ? lightHandler->getEnvPrefilteredMap() : lightHandler->getEnvRadianceMap();
    }
    else
    {
        envRadiance = slangTextureHandler->getZeroImage();
    }
    ImageMap envImages = {
        { HW::ENV_RADIANCE, envRadiance },
        { HW::ENV_IRRADIANCE, lightHandler->getIndirectLighting() ? lightHandler->getEnvIrradianceMap() : slangTextureHandler->getZeroImage() }
    };

    const UniformInputMap& uniformList = getUniformsList();
    for (const auto& env : envImages)
    {
        std::string uniform = env.first;
        ImagePtr image = env.second;

        auto it = uniformList.find(uniform);
        if (image && it != uniformList.end())
        {
            ImageSamplingProperties samplingProperties;
            samplingProperties.uaddressMode = ImageSamplingProperties::AddressMode::PERIODIC;
            samplingProperties.vaddressMode = ImageSamplingProperties::AddressMode::CLAMP;
            samplingProperties.filterType = ImageSamplingProperties::FilterType::LINEAR;

            if (slangTextureHandler->bindImage(image, samplingProperties))
            {
                for (auto& cursor : it->second->slangCursors)
                    slangTextureHandler->bindImage(cursor, image);
            }

            // Bind any associated uniforms.
            if (uniform == HW::ENV_RADIANCE)
            {
                bindUniform(HW::ENV_RADIANCE_MIPS, Value::createValue((int) image->getMaxMipCount()), false);
            }
        }
    }
    bindUniform(HW::REFRACTION_TWO_SIDED, Value::createValue(lightHandler->getRefractionTwoSided()), false);

    // Bind direct lighting properties.
    if (hasUniform(HW::NUM_ACTIVE_LIGHT_SOURCES))
    {
        int lightCount = lightHandler->getDirectLighting() ? (int) lightHandler->getLightSources().size() : 0;
        bindUniform(HW::NUM_ACTIVE_LIGHT_SOURCES, Value::createValue(lightCount));
        LightIdMap idMap = lightHandler->computeLightIdMap(lightHandler->getLightSources());
        size_t index = 0;
        for (NodePtr light : lightHandler->getLightSources())
        {
            auto nodeDef = light->getNodeDef();
            if (!nodeDef)
            {
                continue;
            }

            const std::string prefix = HW::LIGHT_DATA_INSTANCE + "[" + std::to_string(index) + "]";

            // Set light type id
            std::string lightType(prefix + ".type");
            if (hasUniform(lightType))
            {
                unsigned int lightTypeValue = idMap[nodeDef->getName()];
                bindUniform(lightType, Value::createValue((int) lightTypeValue));
            }

            // Set all inputs
            for (const auto& input : light->getInputs())
            {
                // Make sure we have a value to set
                if (input->hasValue())
                {
                    std::string inputName(prefix + "." + input->getName());
                    if (hasUniform(inputName))
                    {
                        if (input->getName() == "direction" && input->hasValue() && input->getValue()->isA<Vector3>())
                        {
                            Vector3 dir = input->getValue()->asA<Vector3>();
                            dir = lightHandler->getLightTransform().transformVector(dir);
                            bindUniform(inputName, Value::createValue(dir));
                        }
                        else
                        {
                            bindUniform(inputName, input->getValue());
                        }
                    }
                }
            }

            ++index;
        }
    }

    // Bind the directional albedo table, if needed.
    ImagePtr albedoTable = lightHandler->getAlbedoTable();
    if (albedoTable && hasUniform(HW::ALBEDO_TABLE))
    {
        ImageSamplingProperties samplingProperties;
        samplingProperties.uaddressMode = ImageSamplingProperties::AddressMode::CLAMP;
        samplingProperties.vaddressMode = ImageSamplingProperties::AddressMode::CLAMP;
        samplingProperties.filterType = ImageSamplingProperties::FilterType::LINEAR;

        auto it = getUniformsList().find(HW::ALBEDO_TABLE);
        if (slangTextureHandler->bindImage(albedoTable, samplingProperties))
        {
            for (auto& cursor : it->second->slangCursors)
                slangTextureHandler->bindImage(cursor, albedoTable);
        }
    }
}

void SlangProgram::bindViewInformation(CameraPtr camera)
{
    if (!isBuilt())
    {
        throw ExceptionRenderError("Cannot bind without a valid program");
    }
    if (!camera)
    {
        throw ExceptionRenderError("Cannot bind without a camera");
    }

    // View position and direction
    bindUniform(HW::VIEW_POSITION, Value::createValue(camera->getViewPosition()), false);
    bindUniform(HW::VIEW_DIRECTION, Value::createValue(camera->getViewDirection()), false);

    // World matrices
    Matrix44 worldInv = camera->getWorldMatrix().getInverse();
    bindUniform(HW::WORLD_MATRIX, Value::createValue(camera->getWorldMatrix()), false);
    bindUniform(HW::WORLD_TRANSPOSE_MATRIX, Value::createValue(camera->getWorldMatrix().getTranspose()), false);
    bindUniform(HW::WORLD_INVERSE_MATRIX, Value::createValue(worldInv), false);
    bindUniform(HW::WORLD_INVERSE_TRANSPOSE_MATRIX, Value::createValue(worldInv.getTranspose()), false);

    // View matrices
    Matrix44 viewInv = camera->getViewMatrix().getInverse();
    bindUniform(HW::VIEW_MATRIX, Value::createValue(camera->getViewMatrix()), false);
    bindUniform(HW::VIEW_TRANSPOSE_MATRIX, Value::createValue(camera->getViewMatrix().getTranspose()), false);
    bindUniform(HW::VIEW_INVERSE_MATRIX, Value::createValue(viewInv), false);
    bindUniform(HW::VIEW_INVERSE_TRANSPOSE_MATRIX, Value::createValue(viewInv.getTranspose()), false);

    // Projection matrices
    Matrix44 projInv = camera->getProjectionMatrix().getInverse();
    bindUniform(HW::PROJ_MATRIX, Value::createValue(camera->getProjectionMatrix()), false);
    bindUniform(HW::PROJ_TRANSPOSE_MATRIX, Value::createValue(camera->getProjectionMatrix().getTranspose()), false);
    bindUniform(HW::PROJ_INVERSE_MATRIX, Value::createValue(projInv), false);
    bindUniform(HW::PROJ_INVERSE_TRANSPOSE_MATRIX, Value::createValue(projInv.getTranspose()), false);

    // View-projection matrix
    Matrix44 viewProj = camera->getViewMatrix() * camera->getProjectionMatrix();
    bindUniform(HW::VIEW_PROJECTION_MATRIX, Value::createValue(viewProj), false);

    // View-projection-world matrix
    Matrix44 worldViewProj = camera->getWorldViewProjMatrix();
    bindUniform(HW::WORLD_VIEW_PROJECTION_MATRIX, Value::createValue(worldViewProj), false);
}

void SlangProgram::bindTimeAndFrame(float time, float frame)
{
    if (!isBuilt())
    {
        throw ExceptionRenderError("Cannot bind time/frame without a valid program");
    }

    bindUniform(HW::TIME, Value::createValue(time), false);
    bindUniform(HW::FRAME, Value::createValue(frame), false);
}

const SlangProgram::VertexInputMap& SlangProgram::updateVertexInputList()
{
    if (!_vertexInputsList.empty())
    {
        return _vertexInputsList;
    }

    if (!_slangReflection)
    {
        throw ExceptionRenderError("Cannot parse for VertexInputs without a valid program");
    }

    std::vector<rhi::VertexStreamDesc> vertexStreams;
    std::vector<rhi::InputElementDesc> inputElements;

    slang::EntryPointReflection* entryPoint = _slangReflection->findEntryPointByName("vertexMain");
    slang::VariableLayoutReflection* entryPointParam = entryPoint->getParameterByIndex(0);
    slang::TypeLayoutReflection* inputs = entryPointParam->getTypeLayout();
    for (unsigned i = 0; i < inputs->getFieldCount(); ++i)
    {
        slang::VariableLayoutReflection* param = inputs->getFieldByIndex(i);
        slang::TypeLayoutReflection* typeLayout = _slangReflection->getTypeLayout(param->getType());

        VertexInputPtr inputPtr = std::make_shared<VertexInput>();
        inputPtr->bufferIndex = i;
        inputPtr->slangTypeLayout = typeLayout;
        inputPtr->valueSemanticName = param->getSemanticName();
        inputPtr->valueSemanticIndex = (uint32_t) param->getSemanticIndex();

        _vertexInputsList[param->getName()] = inputPtr;

        rhi::InputElementDesc desc;
        desc.semanticName = inputPtr->valueSemanticName.c_str();
        desc.semanticIndex = inputPtr->valueSemanticIndex;
        desc.format = getRHIFormat(inputPtr->slangTypeLayout->getType());
        desc.offset = 0;
        desc.bufferSlotIndex = i;
        inputElements.push_back(desc);
        vertexStreams.push_back(rhi::VertexStreamDesc{ (uint32_t) getByteSize(inputPtr->slangTypeLayout), rhi::InputSlotClass::PerVertex, 0 });
    }

    rhi::InputLayoutDesc inputLayoutDesc = {};
    inputLayoutDesc.inputElementCount = (uint32_t) inputElements.size();
    inputLayoutDesc.inputElements = inputElements.data();
    inputLayoutDesc.vertexStreamCount = (uint32_t) vertexStreams.size();
    inputLayoutDesc.vertexStreams = vertexStreams.data();
    _inputLayout = _context->getDevice()->createInputLayout(inputLayoutDesc);

    if (_shader)
    {
        const ShaderStage& vs = _shader->getStage(Stage::VERTEX);
        bool uniformTypeMismatchFound = false;
        StringVec errors;

        const VariableBlock& vertexInputs = vs.getInputBlock(HW::VERTEX_INPUTS);
        if (!vertexInputs.empty())
        {
            for (size_t i = 0; i < vertexInputs.size(); ++i)
            {
                const ShaderPort* v = vertexInputs[i];
                auto inputIt = _vertexInputsList.find(v->getVariable());
                if (inputIt != _vertexInputsList.end())
                {
                    VertexInput* input = inputIt->second.get();
                    if (isEqualType(input->slangTypeLayout->getType(), v->getType()))
                    {
                        input->typeString = v->getType().getName();
                    }
                    else
                    {
                        errors.push_back(
                            "Vertex shader inputs type mismatch in block. Name: \"" + v->getVariable() + "\". Type: \"" + v->getType().getName() + "\". Semantic: \"" + v->getSemantic() + "\". Value: \"" + (v->getValue() ? v->getValue()->getValueString() : "<none>") + "\". SlangType: " + getFullName(input->slangTypeLayout->getType()));
                        uniformTypeMismatchFound = true;
                    }
                }
            }
        }

        // Throw an error if any type mismatches were found
        if (uniformTypeMismatchFound)
        {
            throw ExceptionRenderError("Slang vertex inputs parsing error", errors);
        }
    }

    return _vertexInputsList;
}

const SlangProgram::UniformInputMap& SlangProgram::updateUniformsList()
{
    if (!_uniformList.empty())
    {
        return _uniformList;
    }

    if (!_slangReflection)
    {
        throw ExceptionRenderError("Cannot parse uniforms without a valid program");
    }

    if (!_shader)
    {
        throw ExceptionRenderError("Cannot set default values without the shader present.");
    }

    StringVec errors;

    // Check for any type mismatches between the program and the h/w shader.
    // i.e the type indicated by the HwShader does not match what was generated.
    bool uniformTypeMismatchFound = false;

    auto addUniform = [this](UniformInput&& input)
    {
        if (auto it = _uniformList.find(input.name); it != _uniformList.end())
        {
            if (it->second->slangCursors.back().m_typeLayout->getType() == input.slangCursors.back().m_typeLayout->getType() &&
                it->second->slangByteSize == input.slangByteSize &&
                (it->second->slangDefaultValue == input.slangDefaultValue ||
                 it->second->slangDefaultValue->getValueString() == input.slangDefaultValue->getValueString()))
            {
                it->second->slangCursors.insert(it->second->slangCursors.end(), input.slangCursors.begin(), input.slangCursors.end());
            }
            else
            {
                throw ExceptionRenderError("Uniform variable `" + input.name + "` is defined in multiple stages, but with different values in each.");
            }
        }
        else
        {
            _uniformList[input.name] = std::make_shared<UniformInput>(input);
        }
    };

    auto populateUniformInput = [this, &uniformTypeMismatchFound, &errors, addUniform](rhi::ShaderCursor slangCursor, TypeDesc variableTypeDesc, const std::string& variableName, const ConstValuePtr& variableValue, const ShaderPort* shaderPort, auto& thisFunc) -> void
    {
        slang::TypeLayoutReflection* typeLayout = slangCursor.m_typeLayout;

        // Several structs are considered scalar-like (e.g., SamplerTexture2D), so we need those as well as any non-structs
        if (isScalarLikeType(typeLayout) || typeLayout->getKind() != slang::TypeReflection::Kind::Struct)
        {
            UniformInput input;
            input.name = variableName;
            input.slangCursors = { slangCursor };
            input.slangByteSize = getByteSize(typeLayout);
            input.slangDefaultValue = variableValue;
            input.mxElementPath = shaderPort->getPath();
            input.mxUnit = shaderPort->getUnit();
            input.mxColorspace = shaderPort->getColorSpace();

            if (!isEqualType(typeLayout, variableTypeDesc))
            {
                errors.push_back(
                    "Variable `" + input.name + "` has a mismatch. " + " Slang type: \"" + getFullName(typeLayout) + "\". MaterialX type +\"" + variableTypeDesc.getName() + "\".");
                uniformTypeMismatchFound = true;
            }

            addUniform(std::move(input));
        }
        else
        {
            if (!variableTypeDesc.isStruct())
                throw ExceptionRenderError("Slang type is a struct `" + getFullName(typeLayout->getType()) + "` while the MaterialX type is " + variableTypeDesc.getName());
            auto variableStructMembers = variableTypeDesc.getStructMembers();
            if (variableStructMembers->size() != typeLayout->getFieldCount())
                throw ExceptionRenderError("MaterialX struct has " + std::to_string(variableStructMembers->size()) +
                                           " members, while Slang struct `" +
                                           getFullName(typeLayout->getType()) + "` with " + std::to_string(typeLayout->getFieldCount()));

            // If we're a struct - we need to loop over each member
            auto aggregateValue = std::static_pointer_cast<const AggregateValue>(variableValue);

            for (unsigned i = 0; i < typeLayout->getFieldCount(); ++i)
            {
                const auto& structMember = (*variableStructMembers)[i];

                auto memberVariableSlangCursor = slangCursor[i];
                auto memberVariableName = variableName + "." + structMember.getName();
                auto memberVariableValue = aggregateValue->getMemberValue(i);

                thisFunc(memberVariableSlangCursor, structMember.getType(), memberVariableName, memberVariableValue, shaderPort, thisFunc);
            }
        }
    };

    /// Iterate over all uniforms in a block, either root or a constant buffer.
    auto populateUniforms = [&](rhi::ShaderCursor cursor, const VariableBlockMap& variableBlocks)
    {
        if (cursor.m_typeLayout->getKind() == slang::TypeReflection::Kind::ConstantBuffer)
            cursor = cursor.getDereferenced();

        unsigned fieldIndex = 0;
        for (auto& uniformMap : variableBlocks)
        {
            /// varName must be either portName, or portName + a digit (in case the portName is a reserved keyword)
            auto checkName = [](const std::string& varName, const std::string& portName) -> bool
            {
                if (varName == portName)
                    return true;
                if (varName.size() < portName.size())
                    return false;
                if (varName.compare(0, portName.size(), portName) != 0)
                    return false;
                return std::all_of(varName.begin() + portName.size(), varName.end(), ::isdigit);
            };

            const VariableBlock& uniforms = *uniformMap.second;
            if (uniforms.getName() == HW::LIGHT_DATA)
                continue;

            for (size_t i = 0; i < uniforms.size(); ++i, ++fieldIndex)
            {
                auto shaderPort = uniforms[i];

                slang::VariableLayoutReflection* slangVariable = cursor.m_typeLayout->getFieldByIndex(fieldIndex);
                auto slangCursor = cursor[fieldIndex];
                auto typeDesc = shaderPort->getType();
                auto value = shaderPort->getValue();
                std::string name = slangVariable->getName();
                if (!checkName(name, shaderPort->getName()))
                    throw ExceptionRenderError("Expected variable " + name + " but instead found ShaderPort " + shaderPort->getName() + ".");

                populateUniformInput(slangCursor, typeDesc, name, value, shaderPort, populateUniformInput);
            }
        }

        if (fieldIndex != cursor.m_typeLayout->getFieldCount())
            throw ExceptionRenderError("ShaderCursor has " + std::to_string(cursor.m_typeLayout->getFieldCount()) + " members, while Uniforms have " + std::to_string(fieldIndex) + " ports.");
    };

    rhi::ShaderCursor cursor(_rootObject);
    populateUniforms(cursor[(Stage::PIXEL + "CB").c_str()], _shader->getStage(Stage::PIXEL).getUniformBlocks());
    populateUniforms(cursor[(Stage::VERTEX + "CB").c_str()], _shader->getStage(Stage::VERTEX).getUniformBlocks());

    // Throw an error if any type mismatches were found
    if (uniformTypeMismatchFound)
    {
        throw ExceptionRenderError("Slang uniform parsing error", errors);
    }

    return _uniformList;
}

void SlangProgram::printUniforms(std::ostream& outputStream)
{
    updateUniformsList();
    for (const auto& input : _uniformList)
    {
        string type = input.second->slangDefaultValue ? input.second->slangDefaultValue->getTypeString() : EMPTY_STRING;
        string value = input.second->slangDefaultValue ? input.second->slangDefaultValue->getValueString() : EMPTY_STRING;
        string unit = input.second->mxUnit;
        string colorspace = input.second->mxColorspace;
        outputStream << "Program Uniform: \"" << input.first
                     << "\". SlangType:" << getFullName(input.second->slangCursors.front().m_typeLayout);
        if (!type.empty())
            outputStream << ". TypeString: \"" << type << "\"";
        if (!value.empty())
        {
            outputStream << ". Value: " << value;
            if (!unit.empty())
                outputStream << ". Unit: " << unit;
            if (!colorspace.empty())
                outputStream << ". Colorspace: " << colorspace;
        }
        if (!input.second->mxElementPath.empty())
            outputStream << ". Element Path: \"" << input.second->mxElementPath << "\"";
        outputStream << "." << std::endl;
    }
}

void SlangProgram::printVertexInputs(std::ostream& outputStream)
{
    updateVertexInputList();
    for (const auto& input : _vertexInputsList)
    {
        std::string type = input.second->typeString;
        outputStream << "Vertex input: \"" << input.first
                     << "\". SlangType:" << getFullName(input.second->slangTypeLayout);
        if (!type.empty())
            outputStream << ". TypeString: \"" << type << "\"";
        outputStream << "." << std::endl;
    }
}

MATERIALX_NAMESPACE_END
