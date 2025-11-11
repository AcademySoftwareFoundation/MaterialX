//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
#include <MaterialXGenSlang/SlangShaderGenerator.h>

#include <MaterialXRenderSlang/SlangMaterial.h>
#include <MaterialXRenderSlang/SlangProgram.h>

#include <MaterialXRender/Util.h>
#include <MaterialXFormat/Util.h>

#include <iostream>

MATERIALX_NAMESPACE_BEGIN

#define MX_UNIMPLEMENTED throw ExceptionRenderError("Unimplemented");

SlangMaterialPtr SlangMaterial::create(SlangContextPtr context)
{
    return std::make_shared<SlangMaterial>(std::move(context));
}

SlangMaterial::SlangMaterial(SlangContextPtr context) :
    _context(std::move(context)) { }

SlangMaterial::~SlangMaterial() = default;

/// Load shader source from file.
bool SlangMaterial::loadSource(const FilePath& vertexShaderFile,
                               const FilePath& pixelShaderFile,
                               bool hasTransparency)
{
    _hasTransparency = hasTransparency;

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
    // Here we set new source code on the _program without rebuilding
    // the _hwShader instance. So the _hwShader is not in sync with the
    // _program after this operation.
    _glProgram = SlangProgram::create(_context);
    _glProgram->addStage(Stage::VERTEX, vertexShader);
    _glProgram->addStage(Stage::PIXEL, pixelShader);
    _glProgram->build();

    return true;
}

void SlangMaterial::clearShader()
{
    _hwShader = {};
    _glProgram = {};
}

/// Generate a shader from our currently stored element and
/// the given generator context.
bool SlangMaterial::generateShader(GenContext& context)
{
    if (!_elem)
    {
        return false;
    }

    _hasTransparency = isTransparentSurface(_elem, context.getShaderGenerator().getTarget());

    GenContext materialContext = context;
    materialContext.getOptions().hwTransparency = _hasTransparency;

    // Initialize in case creation fails and throws an exception
    clearShader();

    _hwShader = createShader("Shader", materialContext, _elem);
    if (!_hwShader)
    {
        return false;
    }

    _glProgram = SlangProgram::create(_context);
    _glProgram->setStages(_hwShader);
    _glProgram->build();

    return true;
}

/// Copies shader and API specific generated program from ShaderMaterial to this one.
void SlangMaterial::copyShader(MaterialPtr material)
{
    _hwShader = std::static_pointer_cast<SlangMaterial>(material)->_hwShader;
    _glProgram = std::static_pointer_cast<SlangMaterial>(material)->_glProgram;
}

/// Generate a shader from the given hardware shader.
bool SlangMaterial::generateShader(ShaderPtr hwShader)
{
    _hwShader = hwShader;

    _glProgram = SlangProgram::create(_context);
    _glProgram->setStages(hwShader);
    _glProgram->build();

    return true;
}

/// Bind viewing information for this ShaderMaterial.
void SlangMaterial::bindViewInformation(CameraPtr camera)
{
    if (!_glProgram)
        return;

    _glProgram->bindViewInformation(std::move(camera));
}

/// Bind all images for this ShaderMaterial.
void SlangMaterial::bindImages(ImageHandlerPtr imageHandler,
                               const FileSearchPath& searchPath,
                               bool)
{
    if (!_glProgram)
        return;

    _boundImages.clear();

    FileSearchPath oldSearchPath = imageHandler->getSearchPath();
    imageHandler->setSearchPath(searchPath);

    _glProgram->bindTextures(imageHandler);

    imageHandler->setSearchPath(oldSearchPath);
}

void SlangMaterial::unbindImages(ImageHandlerPtr imageHandler)
{
    /// TODO: As we do not have a concept of unbinding images,
    /// an image is bound until another image is bound to the same place in the descriptor.
    for (ImagePtr image : _boundImages)
    {
        imageHandler->unbindImage(image);
    }
}

ImagePtr SlangMaterial::bindImage(const FilePath& filePath,
                                  const std::string& uniformName,
                                  ImageHandlerPtr imageHandler,
                                  const ImageSamplingProperties& samplingProperties)
{
    if (!_glProgram)
        return {};

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
        return nullptr;

    _glProgram->bindTexture(imageHandler, uniformName, image, samplingProperties);
    return image;
}

/// Bind lights to shader.
void SlangMaterial::bindLighting(LightHandlerPtr lightHandler,
                                 ImageHandlerPtr imageHandler,
                                 const ShadowState& shadowState)
{
    if (!_glProgram)
        return;

    _glProgram->bindLighting(lightHandler, imageHandler);

    // Bind shadow map properties
    if (shadowState.shadowMap && _glProgram->hasUniform(HW::SHADOW_MAP))
    {
        ImageSamplingProperties samplingProperties;
        samplingProperties.uaddressMode = ImageSamplingProperties::AddressMode::CLAMP;
        samplingProperties.vaddressMode = ImageSamplingProperties::AddressMode::CLAMP;
        samplingProperties.filterType = ImageSamplingProperties::FilterType::LINEAR;

        // Bind the shadow map.
        _glProgram->bindTexture(imageHandler, HW::SHADOW_MAP, shadowState.shadowMap, samplingProperties);
        _glProgram->bindUniform(HW::SHADOW_MATRIX, Value::createValue(shadowState.shadowMatrix));
    }

    // Bind ambient occlusion properties.
    if (shadowState.ambientOcclusionMap && _glProgram->hasUniform(HW::AMB_OCC_MAP))
    {
        ImageSamplingProperties samplingProperties;
        samplingProperties.uaddressMode = ImageSamplingProperties::AddressMode::PERIODIC;
        samplingProperties.vaddressMode = ImageSamplingProperties::AddressMode::PERIODIC;
        samplingProperties.filterType = ImageSamplingProperties::FilterType::LINEAR;

        // Bind the ambient occlusion map.
        _glProgram->bindTexture(imageHandler, HW::AMB_OCC_MAP,
                                shadowState.ambientOcclusionMap,
                                samplingProperties);

        _glProgram->bindUniform(HW::AMB_OCC_GAIN, Value::createValue(shadowState.ambientOcclusionGain));
    }
}

void SlangMaterial::bindMesh(MeshPtr mesh)
{
    if (!mesh || !_glProgram)
    {
        return;
    }

    if (!_renderState || !_passEncoder)
    {
        throw ExceptionRenderError("Binding mesh can only be done with set renderState and encoder");
    }

    _glProgram->bind();
    if (_boundMesh && mesh != _boundMesh)
    {
        _glProgram->unbindGeometry();
    }
    _glProgram->bindMesh(mesh, *_renderState);
    _boundMesh = mesh;
}

bool SlangMaterial::bindPartition(MeshPartitionPtr part) const
{
    if (!_glProgram)
    {
        return false;
    }

    _glProgram->bind();
    _renderState->indexBuffer = _glProgram->bindPartition(part);
    return true;
}

void SlangMaterial::drawPartition(MeshPartitionPtr part) const
{
    if (!part)
        return;
    if (!bindPartition(part))
        return;

    rhi::DrawArguments drawArgs = {};
    drawArgs.vertexCount = (uint32_t) part->getIndices().size();

    _passEncoder->setRenderState(*_renderState);
    _passEncoder->drawIndexed(drawArgs);
}

void SlangMaterial::unbindGeometry()
{
    if (_glProgram)
    {
        _glProgram->unbindGeometry();
    }
    _boundMesh = nullptr;
}

/// Return the block of public uniforms for this ShaderMaterial.
VariableBlock* SlangMaterial::getPublicUniforms() const
{
    if (!_hwShader)
        return {};

    ShaderStage& stage = _hwShader->getStage(Stage::PIXEL);
    VariableBlock& block = stage.getUniformBlock(HW::PUBLIC_UNIFORMS);

    return &block;
}

/// Find a public uniform from its MaterialX path.
ShaderPort* SlangMaterial::findUniform(const std::string& path) const
{
    ShaderPort* port = nullptr;
    VariableBlock* publicUniforms = getPublicUniforms();
    if (publicUniforms)
    {
        // Scan block based on path match predicate
        port = publicUniforms->find([path](ShaderPort* port)
        {
            return (port && stringEndsWith(port->getPath(), path));
        });
        if (!port)
        {
            port = publicUniforms->find([path](ShaderPort* port)
            {
                return (port && stringEndsWith(path, port->getName()));
            });
        }

        // Check if the uniform exists in the shader program
        if (port && !_glProgram->getUniformsList().count(port->getVariable()))
        {
            port = nullptr;
        }
    }
    return port;
}

/// Modify the value of the uniform with the given path.
void SlangMaterial::modifyUniform(const std::string& path,
                                  ConstValuePtr value,
                                  std::string valueString)
{
    ShaderPort* uniform = findUniform(path);
    if (!uniform)
    {
        return;
    }

    _glProgram->bindUniform(uniform->getVariable(), value);

    if (valueString.empty())
    {
        valueString = value->getValueString();
    }
    uniform->setValue(uniform->getType().createValueFromStrings(valueString));
    if (_doc)
    {
        ElementPtr element = _doc->getDescendant(uniform->getPath());
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

void SlangMaterial::prepareUsedResources(CameraPtr cam,
                                         GeometryHandlerPtr geometryHandler,
                                         ImageHandlerPtr imageHandler,
                                         LightHandlerPtr lightHandler)
{
    if (!_glProgram)
        return;

    _glProgram->bindViewInformation(cam);
    _glProgram->bindTimeAndFrame();
    _glProgram->bindLighting(lightHandler, imageHandler);
    _glProgram->bindTextures(imageHandler);
}

MATERIALX_NAMESPACE_END
