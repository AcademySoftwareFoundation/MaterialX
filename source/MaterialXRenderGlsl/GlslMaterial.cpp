//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderGlsl/GlslMaterial.h>

#include <MaterialXRenderGlsl/External/Glad/glad.h>
#include <MaterialXRenderGlsl/GLTextureHandler.h>
#include <MaterialXRenderGlsl/GLUtil.h>

#include <MaterialXRender/Util.h>

#include <MaterialXFormat/Util.h>

MATERIALX_NAMESPACE_BEGIN

const std::string DISTANCE_UNIT_TARGET_NAME = "u_distanceUnitTarget";

//
// GlslMaterial methods
//

bool GlslMaterial::loadSource(const FilePath& vertexShaderFile, const FilePath& pixelShaderFile, bool hasTransparency)
{

    std::string vertexShader = readFile(vertexShaderFile);
    if (vertexShader.empty())
    {
        return false;
    }

    std::string pixelShader = readFile(pixelShaderFile);
    if (pixelShader.empty())
    {
        return false;
    }

    // TODO:
    // Here we set new source code on the glProgram without rebuilding 
    // the _hwShader in the definition. So the _hwShader is not in sync with the
    // _glProgram after this operation.
    auto glProgram = GlslProgram::create();
    glProgram->addStage(Stage::VERTEX, vertexShader);
    glProgram->addStage(Stage::PIXEL, pixelShader);

    // Create a new state object as we are ignoring the definition in this case.
    _pState = createState();

    // Set the program in the new state.
    auto glslState = std::static_pointer_cast<GlslShaderMaterialState>(_pState);
    glslState->setProgram(glProgram, hasTransparency);

    return true;
}

void GlslMaterial::clearShader()
{
    _pState = nullptr;
}

bool GlslShaderMaterialState::generateShader(GenContext& context)
{
    if (!_def.elem)
    {
        return false;
    }

    _hasTransparency = isTransparentSurface(_def.elem, context.getShaderGenerator().getTarget());

    // TODO: Just proof-of-concept code, we will need handle the case were generateShader is called twice on the same shared state, with different contexts.
    GenContext materialContext = context;
    materialContext.getOptions().hwTransparency = _hasTransparency;

    // Initialize in case creation fails and throws an exception
    clearShader();

    _hwShader = createShader("Shader", materialContext, _def.elem);
    if (!_hwShader)
    {
        return false;
    }

    _glProgram = GlslProgram::create();
    getProgram()->setStages(_hwShader);

    return true;
}
void GlslShaderMaterialState::clearShader()
{
    _hwShader = nullptr;
    _glProgram = nullptr;
}

bool GlslShaderMaterialState::generateShader(ShaderPtr hwShader)
{
    _hwShader = hwShader;

    _glProgram = GlslProgram::create();
    _glProgram->setStages(hwShader);

    return true;
}

bool GlslMaterial::bindShader() const
{
    if (!getProgram())
    {
        return false;
    }

    if (!getProgram()->hasBuiltData())
    {
        getProgram()->build();
    }
    return getProgram()->bind();
}

void GlslMaterial::bindMesh(MeshPtr mesh)
{
    if (!mesh || !bindShader())
    {
        return;
    }

    if (mesh != _boundMesh)
    {
        getProgram()->unbindGeometry();
    }
    getProgram()->bindMesh(mesh);
    _boundMesh = mesh;
}

bool GlslMaterial::bindPartition(MeshPartitionPtr part) const
{
    if (!bindShader())
    {
        return false;
    }

    getProgram()->bindPartition(part);

    return true;
}

void GlslMaterial::bindViewInformation(CameraPtr camera)
{
    if (!getProgram())
    {
        return;
    }

    getProgram()->bindViewInformation(camera);
}

void GlslMaterial::unbindImages(ImageHandlerPtr imageHandler)
{
    for (ImagePtr image : _boundImages)
    {
        imageHandler->unbindImage(image);
    }
}

void GlslMaterial::bindImages(ImageHandlerPtr imageHandler, const FileSearchPath& searchPath, bool enableMipmaps)
{
    if (!getProgram())
    {
        return;
    }

    _boundImages.clear();

    const VariableBlock* publicUniforms = getPublicUniforms();
    if (!publicUniforms)
    {
        return;
    }
    for (const auto& uniform : publicUniforms->getVariableOrder())
    {
        if (uniform->getType() != Type::FILENAME)
        {
            continue;
        }
        const std::string& uniformVariable = uniform->getVariable();
        std::string filename;
        if (uniform->getValue())
        {
            filename = searchPath.find(uniform->getValue()->getValueString());
        }

        // Extract out sampling properties
        ImageSamplingProperties samplingProperties;
        samplingProperties.setProperties(uniformVariable, *publicUniforms);

        // Set the requested mipmap sampling property,
        samplingProperties.enableMipmaps = enableMipmaps;

        ImagePtr image = bindImage(filename, uniformVariable, imageHandler, samplingProperties);
        if (image)
        {
            _boundImages.push_back(image);
        }
    }
}

ImagePtr GlslMaterial::bindImage(const FilePath& filePath, const std::string& uniformName, ImageHandlerPtr imageHandler,
                                 const ImageSamplingProperties& samplingProperties)
{
    if (!getProgram())
    {
        return nullptr;
    }

    // Create a filename resolver for geometric properties.
    StringResolverPtr resolver = StringResolver::create();
    if (!getUdim().empty())
    {
        resolver->setUdimString(getUdim());
    }
    imageHandler->setFilenameResolver(resolver);

    // Acquire the given image.
    ImagePtr image = imageHandler->acquireImage(filePath, samplingProperties.defaultColor);
    if (!image)
    {
        return nullptr;
    }

    // Bind the image and set its sampling properties.
    if (imageHandler->bindImage(image, samplingProperties))
    {
        GLTextureHandlerPtr textureHandler = std::static_pointer_cast<GLTextureHandler>(imageHandler);
        int textureLocation = textureHandler->getBoundTextureLocation(image->getResourceId());
        if (textureLocation >= 0)
        {
            getProgram()->bindUniform(uniformName, Value::createValue(textureLocation), false);
            return image;
        }
    }
    return nullptr;
}

void GlslMaterial::bindUniformOverrides()
{
    if (!_override)
        return;

    // TODO: This is proof-of-concept code, in production this should probably use uniform blocks, or at least precompute the uniform locations.
    for (int i = 0; i < _override->getPropertyCount(); i++) {
        auto input = _override->getPropertyInput(i);
        auto value = _override->getValue(i);
        // Compute a variable name from property input using an underscore seperator.
        string variableName = input->getParent()->getName() + "_" + input->getName();
        // Set the uniform if it exists.
        if (getProgram()->hasUniform(variableName))
            getProgram()->bindUniform(variableName, value);
    }
}

void GlslMaterial::bindLighting(LightHandlerPtr lightHandler, ImageHandlerPtr imageHandler, const ShadowState& shadowState)
{
    if (!getProgram())
    {
        return;
    }

    // Bind environment and local lighting.
    getProgram()->bindLighting(lightHandler, imageHandler);

    // Bind shadow map properties
    if (shadowState.shadowMap && getProgram()->hasUniform(HW::SHADOW_MAP))
    {
        ImageSamplingProperties samplingProperties;
        samplingProperties.uaddressMode = ImageSamplingProperties::AddressMode::CLAMP;
        samplingProperties.vaddressMode = ImageSamplingProperties::AddressMode::CLAMP;
        samplingProperties.filterType = ImageSamplingProperties::FilterType::LINEAR;

        // Bind the shadow map.
        if (imageHandler->bindImage(shadowState.shadowMap, samplingProperties))
        {
            GLTextureHandlerPtr textureHandler = std::static_pointer_cast<GLTextureHandler>(imageHandler);
            int textureLocation = textureHandler->getBoundTextureLocation(shadowState.shadowMap->getResourceId());
            if (textureLocation >= 0)
            {
                getProgram()->bindUniform(HW::SHADOW_MAP, Value::createValue(textureLocation));
            }
        }
        getProgram()->bindUniform(HW::SHADOW_MATRIX, Value::createValue(shadowState.shadowMatrix));
    }

    // Bind ambient occlusion properties.
    if (shadowState.ambientOcclusionMap && getProgram()->hasUniform(HW::AMB_OCC_MAP))
    {
        ImageSamplingProperties samplingProperties;
        samplingProperties.uaddressMode = ImageSamplingProperties::AddressMode::PERIODIC;
        samplingProperties.vaddressMode = ImageSamplingProperties::AddressMode::PERIODIC;
        samplingProperties.filterType = ImageSamplingProperties::FilterType::LINEAR;

        // Bind the ambient occlusion map.
        if (imageHandler->bindImage(shadowState.ambientOcclusionMap, samplingProperties))
        {
            GLTextureHandlerPtr textureHandler = std::static_pointer_cast<GLTextureHandler>(imageHandler);
            int textureLocation = textureHandler->getBoundTextureLocation(shadowState.ambientOcclusionMap->getResourceId());
            if (textureLocation >= 0)
            {
                getProgram()->bindUniform(HW::AMB_OCC_MAP, Value::createValue(textureLocation));
            }
        }
        getProgram()->bindUniform(HW::AMB_OCC_GAIN, Value::createValue(shadowState.ambientOcclusionGain));
    }
}

void GlslMaterial::drawPartition(MeshPartitionPtr part) const
{
    if (!part || !bindPartition(part))
    {
        return;
    }
    MeshIndexBuffer& indexData = part->getIndices();
    glDrawElements(GL_TRIANGLES, (GLsizei) indexData.size(), GL_UNSIGNED_INT, (void*) 0);
    checkGlErrors("after draw partition");
}

void GlslMaterial::unbindGeometry()
{
    if (!_boundMesh)
    {
        return;
    }

    if (bindShader())
    {
        getState()->getProgram()->unbindGeometry();
    }
    _boundMesh = nullptr;
}

VariableBlock* GlslMaterial::getPublicUniforms() const
{
    if (!_pState)
    {
        return nullptr;
    }

    ShaderStage& stage = _pState->getShader()->getStage(Stage::PIXEL);
    VariableBlock& block = stage.getUniformBlock(HW::PUBLIC_UNIFORMS);

    return &block;
}

GlslShaderMaterialStatePtr GlslMaterial::getState() const
{
    return std::static_pointer_cast<GlslShaderMaterialState>(_pState);
}

ShaderPort* GlslMaterial::findUniform(const std::string& path) const
{
    ShaderPort* port = nullptr;
    VariableBlock* publicUniforms = getPublicUniforms();
    if (publicUniforms)
    {
        // Scan block based on path match predicate
        port = publicUniforms->find(
            [path](ShaderPort* port)
            {
                return (port && stringEndsWith(port->getPath(), path));
            });

        // Check if the uniform exists in the shader program
        if (port && !getState()->getProgram()->getUniformsList().count(port->getVariable()))
        {
            port = nullptr;
        }
    }
    return port;
}

void GlslMaterial::modifyUniform(const std::string& path, ConstValuePtr value, std::string valueString)
{
    if (!bindShader())
    {
        return;
    }

    ShaderPort* uniform = findUniform(path);
    if (!uniform)
    {
        return;
    }

    getProgram()->bindUniform(uniform->getVariable(), value);

    if (valueString.empty())
    {
        valueString = value->getValueString();
    }
    uniform->setValue(Value::createValueFromStrings(valueString, uniform->getType()->getName()));
    if (_def.doc)
    {
        ElementPtr element = _def.doc->getDescendant(uniform->getPath());
        if (element)
        {
            ValueElementPtr valueElement = element->asA<ValueElement>();
            if (valueElement)
            {
                valueElement->setValueString(valueString);
            }
        }
    }
}

MATERIALX_NAMESPACE_END
