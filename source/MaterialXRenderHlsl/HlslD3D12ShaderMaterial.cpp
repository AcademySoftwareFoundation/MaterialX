//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslD3D12ShaderMaterial.h>
#include <MaterialXRenderHlsl/HlslRenderUtil.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXRender/ShaderRenderer.h>
#include <MaterialXRender/Util.h>
#include <MaterialXFormat/Util.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

// Write uniforms into whichever stage of an HlslD3D12Material declares them.
class MaterialUniformWriter : public HlslUniformWriter
{
  public:
    explicit MaterialUniformWriter(HlslD3D12Material& material) :
        _material(material)
    {
    }

    bool writeVertexUniform(const std::string& name, const void* data, std::size_t count) override
    {
        return _material.patchVariable(HlslD3D12Material::Stage::Vertex, name, data, count);
    }

    bool writePixelUniform(const std::string& name, const void* data, std::size_t count) override
    {
        return _material.patchVariable(HlslD3D12Material::Stage::Pixel, name, data, count);
    }

    bool writePixelArrayMember(const std::string& arrayName, std::size_t index,
                               const std::string& memberName, const void* data, std::size_t count) override
    {
        return _material.patchArrayMember(HlslD3D12Material::Stage::Pixel, arrayName, index, memberName, data, count);
    }

  private:
    HlslD3D12Material& _material;
};

bool writeUniform(HlslD3D12Material& material, const std::string& name, ConstValuePtr value)
{
    uint8_t buf[64];
    const std::size_t n = packHlslUniformValue(value, buf);
    if (n == 0)
        return false;
    const bool pixel = material.patchVariable(HlslD3D12Material::Stage::Pixel, name, buf, n);
    const bool vertex = material.patchVariable(HlslD3D12Material::Stage::Vertex, name, buf, n);
    return pixel || vertex;
}

HlslProgramPtr createProgram()
{
    // DXC at shader model 6.0 when available, as in HlslD3D12Renderer.
    HlslProgramPtr program = HlslProgram::create();
    if (HlslProgram::isDxcAvailable())
    {
        program->setCompilerBackend(HlslCompilerBackend::Dxc);
        program->setShaderModel("6_0");
    }
    return program;
}

ImageSamplingProperties clampedLinearSampling()
{
    ImageSamplingProperties sp;
    sp.uaddressMode = ImageSamplingProperties::AddressMode::CLAMP;
    sp.vaddressMode = ImageSamplingProperties::AddressMode::CLAMP;
    sp.filterType = ImageSamplingProperties::FilterType::LINEAR;
    return sp;
}

} // namespace

HlslD3D12ShaderMaterial::HlslD3D12ShaderMaterial(HlslD3D12ContextPtr context, HlslD3D12GeometryCachePtr geometryCache) :
    ShaderMaterial(),
    _context(std::move(context)),
    _geometryCache(std::move(geometryCache))
{
}

HlslD3D12ShaderMaterial::~HlslD3D12ShaderMaterial() = default;

bool HlslD3D12ShaderMaterial::loadSource(const FilePath& vertexShaderFile, const FilePath& pixelShaderFile,
                                         bool hasTransparency)
{
    _hasTransparency = hasTransparency;

    std::string vertexShader = readFile(vertexShaderFile);
    std::string pixelShader = readFile(pixelShaderFile);
    if (vertexShader.empty() || pixelShader.empty())
        return false;

    // As in GlslMaterial, the loaded sources replace the program without
    // rebuilding the generated shader, which keeps providing the uniform
    // values.
    _vertexSource = vertexShader;
    _pixelSource = pixelShader;
    _program = nullptr;
    _material = nullptr;
    return true;
}

void HlslD3D12ShaderMaterial::clearShader()
{
    _hwShader = nullptr;
    _program = nullptr;
    _material = nullptr;
    _vertexSource.clear();
    _pixelSource.clear();
}

bool HlslD3D12ShaderMaterial::generateShader(GenContext& context)
{
    if (!_elem)
        return false;

    _hasTransparency = isTransparentSurface(_elem, context.getShaderGenerator().getTarget());

    GenContext materialContext = context;
    materialContext.getOptions().hwTransparency = _hasTransparency;

    // Initialize in case creation fails and throws an exception.
    clearShader();

    _hwShader = createShader("Shader", materialContext, _elem);
    return _hwShader != nullptr;
}

bool HlslD3D12ShaderMaterial::generateShader(ShaderPtr hwShader)
{
    clearShader();
    _hwShader = hwShader;
    return _hwShader != nullptr;
}

void HlslD3D12ShaderMaterial::copyShader(MaterialPtr material)
{
    auto other = std::static_pointer_cast<HlslD3D12ShaderMaterial>(material);
    _hwShader = other->_hwShader;
    _vertexSource = other->_vertexSource;
    _pixelSource = other->_pixelSource;
    _program = other->_program;
    _material = other->_material;
}

bool HlslD3D12ShaderMaterial::bindShader() const
{
    if (_material)
        return true;
    if (!_hwShader && (_vertexSource.empty() || _pixelSource.empty()))
        return false;

    _program = createProgram();
    const bool built = _vertexSource.empty() ? _program->build(_hwShader)
                                             : _program->build(_vertexSource, _pixelSource);
    if (!built)
    {
        const std::string log = _program->getCompileLog();
        _program = nullptr;
        throw ExceptionRenderError("HlslD3D12ShaderMaterial: HLSL compile failed", { log });
    }
    _material = HlslD3D12Material::create(_context, _program);

    // Start from the default values of every uniform the generator
    // declared, as GlslProgram does when a program is first bound.
    if (_hwShader)
    {
        for (const std::string& stageName : { Stage::VERTEX, Stage::PIXEL })
        {
            const ShaderStage& stage = _hwShader->getStage(stageName);
            for (const auto& it : stage.getUniformBlocks())
            {
                const VariableBlock& block = *it.second;
                for (size_t i = 0; i < block.size(); ++i)
                {
                    const ShaderPort* port = block[i];
                    if (port && port->getValue() && port->getType() != Type::FILENAME)
                        writeUniform(*_material, port->getVariable(), port->getValue());
                }
            }
        }
    }

    // The generator declares the transform matrices without values, and
    // screen-space passes draw without binding a camera, so start from
    // identity transforms.
    MaterialUniformWriter writer(*_material);
    bindHlslCamera(writer, Camera::create());
    return true;
}

void HlslD3D12ShaderMaterial::bindViewInformation(CameraPtr camera)
{
    if (!camera || !bindShader())
        return;

    // The matrices used by the generated vertex stage, with the D3D depth
    // range and matrix layout.
    MaterialUniformWriter writer(*_material);
    bindHlslCamera(writer, camera);

    // The remaining matrices keep the conventions of the shared GLSL
    // library code that reads them, as bound by GlslProgram.
    const Matrix44 worldInv = camera->getWorldMatrix().getInverse();
    const Matrix44 viewInv = camera->getViewMatrix().getInverse();
    const Matrix44 projInv = camera->getProjectionMatrix().getInverse();
    const std::pair<const std::string*, Matrix44> matrices[] = {
        { &HW::WORLD_TRANSPOSE_MATRIX, camera->getWorldMatrix().getTranspose() },
        { &HW::WORLD_INVERSE_MATRIX, worldInv },
        { &HW::VIEW_MATRIX, camera->getViewMatrix() },
        { &HW::VIEW_TRANSPOSE_MATRIX, camera->getViewMatrix().getTranspose() },
        { &HW::VIEW_INVERSE_MATRIX, viewInv },
        { &HW::VIEW_INVERSE_TRANSPOSE_MATRIX, viewInv.getTranspose() },
        { &HW::PROJ_MATRIX, camera->getProjectionMatrix() },
        { &HW::PROJ_TRANSPOSE_MATRIX, camera->getProjectionMatrix().getTranspose() },
        { &HW::PROJ_INVERSE_MATRIX, projInv },
        { &HW::PROJ_INVERSE_TRANSPOSE_MATRIX, projInv.getTranspose() },
        { &HW::WORLD_VIEW_PROJECTION_MATRIX, camera->getWorldViewProjMatrix() },
    };
    for (const auto& m : matrices)
        writeUniform(*_material, *m.first, Value::createValue(m.second));
}

void HlslD3D12ShaderMaterial::unbindImages(ImageHandlerPtr)
{
    _boundImages.clear();
}

void HlslD3D12ShaderMaterial::bindImages(ImageHandlerPtr imageHandler, const FileSearchPath& searchPath, bool enableMipmaps)
{
    if (!bindShader())
        return;

    _boundImages.clear();

    const VariableBlock* publicUniforms = getPublicUniforms();
    if (!publicUniforms)
        return;
    for (const auto& uniform : publicUniforms->getVariableOrder())
    {
        if (uniform->getType() != Type::FILENAME)
            continue;
        const std::string& uniformVariable = uniform->getVariable();
        std::string filename;
        if (uniform->getValue())
            filename = searchPath.find(uniform->getValue()->getValueString());

        ImageSamplingProperties samplingProperties;
        samplingProperties.setProperties(uniformVariable, *publicUniforms);
        samplingProperties.enableMipmaps = enableMipmaps;

        ImagePtr image = bindImage(filename, uniformVariable, imageHandler, samplingProperties);
        if (image)
            _boundImages.push_back(image);
    }
}

ImagePtr HlslD3D12ShaderMaterial::bindImage(const FilePath& filePath, const std::string& uniformName,
                                            ImageHandlerPtr imageHandler,
                                            const ImageSamplingProperties& samplingProperties)
{
    HlslD3D12TextureHandlerPtr textureHandler = std::dynamic_pointer_cast<HlslD3D12TextureHandler>(imageHandler);
    if (!textureHandler || !bindShader())
        return nullptr;

    // Create a filename resolver for geometric properties.
    StringResolverPtr resolver = StringResolver::create();
    if (!getUdim().empty())
        resolver->setUdimString(getUdim());
    imageHandler->setFilenameResolver(resolver);

    ImagePtr image = imageHandler->acquireImage(filePath, samplingProperties.defaultColor);
    if (!image)
        return nullptr;
    return bindTexture(uniformName, image, textureHandler, samplingProperties) ? image : nullptr;
}

bool HlslD3D12ShaderMaterial::bindTexture(const std::string& uniformName, ImagePtr image,
                                          HlslD3D12TextureHandlerPtr textureHandler,
                                          const ImageSamplingProperties& samplingProperties)
{
    if (!image || !textureHandler || !bindShader())
        return false;
    if (!textureHandler->bindImage(image, samplingProperties))
        return false;

    const uint64_t srv = textureHandler->getBoundSrv(image->getResourceId());
    const uint64_t sampler = textureHandler->getBoundSampler(image->getResourceId());
    const std::string texName = uniformName + ".tex";
    const std::string sampName = uniformName + ".samp";
    bool found = false;
    for (const HlslResourceBinding& b : _material->getPixelBindings())
    {
        if (b.space != 0)
            continue;
        if (b.type == HlslResourceType::Texture && b.name == texName)
        {
            _material->setTexture(b.slot, srv);
            found = true;
        }
        else if (b.type == HlslResourceType::Sampler && b.name == sampName)
        {
            _material->setSampler(b.slot, sampler);
            found = true;
        }
    }
    return found;
}

void HlslD3D12ShaderMaterial::bindLighting(LightHandlerPtr lightHandler, ImageHandlerPtr imageHandler,
                                           const ShadowState& shadowState)
{
    if (!lightHandler || !bindShader())
        return;
    HlslD3D12TextureHandlerPtr textureHandler = std::dynamic_pointer_cast<HlslD3D12TextureHandler>(imageHandler);

    MaterialUniformWriter writer(*_material);
    bindHlslLightingScalars(writer, lightHandler);
    bindHlslLightSources(writer, lightHandler);

    if (textureHandler)
    {
        // Environment maps wrap horizontally and clamp at the poles, as in
        // GlslProgram::bindLighting.
        ImageSamplingProperties envSampling;
        envSampling.uaddressMode = ImageSamplingProperties::AddressMode::PERIODIC;
        envSampling.vaddressMode = ImageSamplingProperties::AddressMode::CLAMP;
        envSampling.filterType = ImageSamplingProperties::FilterType::LINEAR;
        bindHlslEnvironmentImages(lightHandler, imageHandler,
                                  [&](const std::string& name, ImagePtr image, const ImageSamplingProperties*)
                                  {
                                      return bindTexture(name, image, textureHandler, envSampling);
                                  });

        if (lightHandler->getAlbedoTable())
            bindTexture(HW::ALBEDO_TABLE, lightHandler->getAlbedoTable(), textureHandler, clampedLinearSampling());

        if (shadowState.shadowMap)
        {
            bindTexture(HW::SHADOW_MAP, shadowState.shadowMap, textureHandler, clampedLinearSampling());
            bindUniform(HW::SHADOW_MATRIX, Value::createValue(shadowState.shadowMatrix));
        }

        if (shadowState.ambientOcclusionMap)
        {
            ImageSamplingProperties aoSampling;
            aoSampling.uaddressMode = ImageSamplingProperties::AddressMode::PERIODIC;
            aoSampling.vaddressMode = ImageSamplingProperties::AddressMode::PERIODIC;
            aoSampling.filterType = ImageSamplingProperties::FilterType::LINEAR;
            bindTexture(HW::AMB_OCC_MAP, shadowState.ambientOcclusionMap, textureHandler, aoSampling);
            bindUniform(HW::AMB_OCC_GAIN, Value::createValue(shadowState.ambientOcclusionGain));
        }
    }
}

void HlslD3D12ShaderMaterial::bindMesh(MeshPtr mesh)
{
    if (!mesh || !bindShader())
        return;
    _boundMesh = mesh;
}

bool HlslD3D12ShaderMaterial::bindPartition(MeshPartitionPtr part) const
{
    return part && _boundMesh && bindShader();
}

void HlslD3D12ShaderMaterial::drawPartition(MeshPartitionPtr part) const
{
    if (!bindPartition(part))
        return;

    ID3D12GraphicsCommandList* list = _context->getFrameCommandList();
    if (!list)
        throw ExceptionRenderError("HlslD3D12ShaderMaterial::drawPartition: no frame is being recorded.");

    HlslD3D12DrawBuffers buffers;
    if (!_geometryCache->getDrawBuffers(_boundMesh, part, _material->getVertexLayout(),
                                        _material->getVertexStride(), buffers))
    {
        return;
    }

    // Public uniform values live on the generated shader, so that edits
    // made through modifyUniform() apply to the next draw.
    if (_hwShader)
    {
        MaterialUniformWriter writer(*_material);
        bindHlslMaterialUniforms(writer, _hwShader);
    }

    _material->bind(list, _context->getRenderTargetFormat(), _context->getDepthFormat(),
                    _context->getRenderState());
    HlslD3D12GeometryCache::draw(list, buffers);
}

void HlslD3D12ShaderMaterial::unbindGeometry()
{
    _boundMesh = nullptr;
}

VariableBlock* HlslD3D12ShaderMaterial::getPublicUniforms() const
{
    if (!_hwShader)
        return nullptr;

    ShaderStage& stage = _hwShader->getStage(Stage::PIXEL);
    return &stage.getUniformBlock(HW::PUBLIC_UNIFORMS);
}

ShaderPort* HlslD3D12ShaderMaterial::findUniform(const std::string& path) const
{
    VariableBlock* publicUniforms = getPublicUniforms();
    if (!publicUniforms)
        return nullptr;

    // Scan block based on path match predicate.
    ShaderPort* shaderPort = publicUniforms->find([path](ShaderPort* port)
    {
        return (port && stringEndsWith(port->getPath(), path));
    });
    if (!shaderPort)
    {
        shaderPort = publicUniforms->find([path](ShaderPort* port)
        {
            return (port && stringEndsWith(path, port->getName()));
        });
    }

    // As in GlslMaterial, only report uniforms the compiled program uses.
    if (shaderPort && _material)
    {
        const std::string& variable = shaderPort->getVariable();
        HlslUniformLocation location;
        bool used = _material->getPixelReflection().findVariable(variable, location) ||
                    _material->getVertexReflection().findVariable(variable, location);
        for (const HlslResourceBinding& b : _material->getPixelBindings())
            used = used || (b.name == variable + ".tex");
        if (!used)
            shaderPort = nullptr;
    }
    return shaderPort;
}

void HlslD3D12ShaderMaterial::modifyUniform(const std::string& path, ConstValuePtr value, std::string valueString)
{
    if (!bindShader())
        return;

    ShaderPort* uniform = findUniform(path);
    if (!uniform)
        return;

    if (valueString.empty())
        valueString = value->getValueString();
    uniform->setValue(uniform->getType().createValueFromStrings(valueString));
    if (_doc)
    {
        ElementPtr element = _doc->getDescendant(uniform->getPath());
        if (element)
        {
            ValueElementPtr valueElement = element->asA<ValueElement>();
            if (valueElement)
                valueElement->setValueString(valueString);
        }
    }
}

bool HlslD3D12ShaderMaterial::bindUniform(const std::string& name, ConstValuePtr value)
{
    return bindShader() && writeUniform(*_material, name, value);
}

void HlslD3D12ShaderMaterial::bindTimeAndFrame(float time, float frame)
{
    bindUniform(HW::TIME, Value::createValue(time));
    bindUniform(HW::FRAME, Value::createValue(frame));
}

MATERIALX_NAMESPACE_END
