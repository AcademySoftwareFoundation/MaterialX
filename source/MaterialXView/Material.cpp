#include <MaterialXView/Material.h>

#include <MaterialXRenderGlsl/External/Glad/glad.h>
#include <MaterialXRenderGlsl/GLTextureHandler.h>
#include <MaterialXRenderGlsl/GLUtil.h>

#include <MaterialXRender/Util.h>

#include <MaterialXFormat/Util.h>

//
// Material methods
//

bool Material::loadSource(const mx::FilePath& vertexShaderFile, const mx::FilePath& pixelShaderFile, bool hasTransparency)
{
    _hasTransparency = hasTransparency;

    std::string vertexShader = mx::readFile(vertexShaderFile);
    if (vertexShader.empty())
    {
        return false;
    }

    std::string pixelShader = mx::readFile(pixelShaderFile);
    if (pixelShader.empty())
    {
        return false;
    }

    // TODO:
    // Here we set new source code on the _glProgram without rebuilding 
    // the _hwShader instance. So the _hwShader is not in sync with the
    // _glProgram after this operation.
    _glProgram = mx::GlslProgram::create();
    _glProgram->addStage(mx::Stage::VERTEX, vertexShader);
    _glProgram->addStage(mx::Stage::PIXEL, pixelShader);
    _glProgram->build();

    updateUniformsList();

    return true;
}

void Material::updateUniformsList()
{
    _uniformVariable.clear();
    if (!_glProgram)
    {
        return;
    }

    for (const auto& pair : _glProgram->getUniformsList())
    {
        _uniformVariable.insert(pair.first);
    }
}

void Material::clearShader()
{
    _hwShader = nullptr;
    _glProgram = nullptr;
    _uniformVariable.clear();
}

bool Material::generateShader(mx::GenContext& context)
{
    if (!_elem)
    {
        return false;
    }

    _hasTransparency = mx::isTransparentSurface(_elem, context.getShaderGenerator().getTarget());

    mx::GenContext materialContext = context;
    materialContext.getOptions().hwTransparency = _hasTransparency;

    // Initialize in case creation fails and throws an exception
    clearShader();

    _hwShader = createShader("Shader", materialContext, _elem);
    if (!_hwShader)
    {
        return false;
    }

    _glProgram = mx::GlslProgram::create();
    _glProgram->setStages(_hwShader);
    _glProgram->build();

    updateUniformsList();

    return true;
}

bool Material::generateShader(mx::ShaderPtr hwShader)
{
    _hwShader = hwShader;

    _glProgram = mx::GlslProgram::create();
    _glProgram->setStages(hwShader);
    _glProgram->build();

    updateUniformsList();

    return true;
}

bool Material::generateEnvironmentShader(mx::GenContext& context,
                                         const mx::FilePath& filename,
                                         mx::DocumentPtr stdLib,
                                         const mx::FilePath& imagePath)
{
    // Read in the environment nodegraph. 
    mx::DocumentPtr doc = mx::createDocument();
    doc->importLibrary(stdLib);
    mx::DocumentPtr envDoc = mx::createDocument();
    mx::readFromXmlFile(envDoc, filename);
    doc->importLibrary(envDoc);

    mx::NodeGraphPtr envGraph = doc->getNodeGraph("environmentDraw");
    if (!envGraph)
    {
        return false;
    }
    mx::NodePtr image = envGraph->getNode("envImage");
    if (!image)
    {
        return false;
    }
    image->setInputValue("file", imagePath.asString(), mx::FILENAME_TYPE_STRING);
    mx::OutputPtr output = envGraph->getOutput("out");
    if (!output)
    {
        return false;
    }

    // Create the shader.
    std::string shaderName = "__ENV_SHADER__";
    _hwShader = createShader(shaderName, context, output); 
    if (!_hwShader)
    {
        return false;
    }
    return generateShader(_hwShader);
}

void Material::bindShader()
{
    if (_glProgram)
    {
        _glProgram->bind();
    }
}

void Material::bindMesh(mx::MeshPtr mesh)
{
    if (!mesh || !_glProgram)
    {
        return;
    }

    _glProgram->bind();
    if (_boundMesh && mesh->getName() != _boundMesh->getName())
    {
        _glProgram->unbindGeometry();
    }
    _glProgram->bindMesh(mesh);
    _boundMesh = mesh;
}

bool Material::bindPartition(mx::MeshPartitionPtr part) const
{
    if (!_glProgram)
    {
        return false;
    }

    _glProgram->bind();
    _glProgram->bindPartition(part);

    return true;
}

void Material::bindViewInformation(mx::CameraPtr camera)
{
    if (!_glProgram)
    {
        return;
    }

    _glProgram->bindViewInformation(camera);
}

void Material::unbindImages(mx::ImageHandlerPtr imageHandler)
{
    for (mx::ImagePtr image : _boundImages)
    {
        imageHandler->unbindImage(image);
    }
}

void Material::bindImages(mx::ImageHandlerPtr imageHandler, const mx::FileSearchPath& searchPath, bool enableMipmaps)
{
    if (!_glProgram)
    {
        return;
    }

    _boundImages.clear();

    const mx::VariableBlock* publicUniforms = getPublicUniforms();
    if (!publicUniforms)
    {
        return;
    }
    for (const auto& uniform : publicUniforms->getVariableOrder())
    {
        if (uniform->getType() != mx::Type::FILENAME)
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
        mx::ImageSamplingProperties samplingProperties;
        samplingProperties.setProperties(uniformVariable, *publicUniforms);

        // Set the requested mipmap sampling property,
        samplingProperties.enableMipmaps = enableMipmaps;

        mx::ImagePtr image = bindImage(filename, uniformVariable, imageHandler, samplingProperties);
        if (image)
        {
            _boundImages.push_back(image);
        }
    }
}

mx::ImagePtr Material::bindImage(const mx::FilePath& filePath, const std::string& uniformName, mx::ImageHandlerPtr imageHandler,
                                 const mx::ImageSamplingProperties& samplingProperties)
{
    if (!_glProgram)
    {
        return nullptr;
    }

    // Create a filename resolver for geometric properties.
    mx::StringResolverPtr resolver = mx::StringResolver::create();
    if (!getUdim().empty())
    {
        resolver->setUdimString(getUdim());
    }
    imageHandler->setFilenameResolver(resolver);

    // Acquire the given image.
    mx::ImagePtr image = imageHandler->acquireImage(filePath);
    if (!image)
    {
        return nullptr;
    }

    // Bind the image and set its sampling properties.
    if (imageHandler->bindImage(image, samplingProperties))
    {
        mx::GLTextureHandlerPtr textureHandler = std::static_pointer_cast<mx::GLTextureHandler>(imageHandler);
        int textureLocation = textureHandler->getBoundTextureLocation(image->getResourceId());
        if (textureLocation >= 0)
        {
            _glProgram->bindUniform(uniformName, mx::Value::createValue(textureLocation), false);
            return image;
        }
    }
    return nullptr;
}

void Material::bindLighting(mx::LightHandlerPtr lightHandler, mx::ImageHandlerPtr imageHandler, const ShadowState& shadowState)
{
    if (!_glProgram)
    {
        return;
    }

    // Bind environment and local lighting.
    _glProgram->bindLighting(lightHandler, imageHandler);

    // Bind shadow map properties
    if (shadowState.shadowMap && _glProgram->hasUniform(mx::HW::SHADOW_MAP))
    {
        mx::ImageSamplingProperties samplingProperties;
        samplingProperties.uaddressMode = mx::ImageSamplingProperties::AddressMode::CLAMP;
        samplingProperties.vaddressMode = mx::ImageSamplingProperties::AddressMode::CLAMP;
        samplingProperties.filterType = mx::ImageSamplingProperties::FilterType::LINEAR;

        // Bind the shadow map.
        if (imageHandler->bindImage(shadowState.shadowMap, samplingProperties))
        {
            mx::GLTextureHandlerPtr textureHandler = std::static_pointer_cast<mx::GLTextureHandler>(imageHandler);
            int textureLocation = textureHandler->getBoundTextureLocation(shadowState.shadowMap->getResourceId());
            if (textureLocation >= 0)
            {
                _glProgram->bindUniform(mx::HW::SHADOW_MAP, mx::Value::createValue(textureLocation));
            }
        }
        _glProgram->bindUniform(mx::HW::SHADOW_MATRIX, mx::Value::createValue(shadowState.shadowMatrix));
    }

    // Bind ambient occlusion properties.
    if (shadowState.ambientOcclusionMap && _glProgram->hasUniform(mx::HW::AMB_OCC_MAP))
    {
        mx::ImageSamplingProperties samplingProperties;
        samplingProperties.uaddressMode = mx::ImageSamplingProperties::AddressMode::PERIODIC;
        samplingProperties.vaddressMode = mx::ImageSamplingProperties::AddressMode::PERIODIC;
        samplingProperties.filterType = mx::ImageSamplingProperties::FilterType::LINEAR;

        // Bind the ambient occlusion map.
        if (imageHandler->bindImage(shadowState.ambientOcclusionMap, samplingProperties))
        {
            mx::GLTextureHandlerPtr textureHandler = std::static_pointer_cast<mx::GLTextureHandler>(imageHandler);
            int textureLocation = textureHandler->getBoundTextureLocation(shadowState.ambientOcclusionMap->getResourceId());
            if (textureLocation >= 0)
            {
                _glProgram->bindUniform(mx::HW::AMB_OCC_MAP, mx::Value::createValue(textureLocation));
            }
        }
        _glProgram->bindUniform(mx::HW::AMB_OCC_GAIN, mx::Value::createValue(shadowState.ambientOcclusionGain));
    }
}

void Material::bindUnits(mx::UnitConverterRegistryPtr& registry, const mx::GenContext& context)
{
    static std::string DISTANCE_UNIT_TARGET_NAME = "u_distanceUnitTarget";

    _glProgram->bind();

    mx::ShaderPort* port = nullptr;
    mx::VariableBlock* publicUniforms = getPublicUniforms();
    if (publicUniforms)
    {
        // Scan block based on unit name match predicate
        port = publicUniforms->find(
            [](mx::ShaderPort* port)
        {
            return (port && (port->getName() == DISTANCE_UNIT_TARGET_NAME));
        });

        // Check if the uniform exists in the shader program
        if (port && !_uniformVariable.count(port->getVariable()))
        {
            port = nullptr;
        }
    }

    if (port)
    {
        int intPortValue = registry->getUnitAsInteger(context.getOptions().targetDistanceUnit);
        if (intPortValue >= 0)
        {
            port->setValue(mx::Value::createValue(intPortValue));
            if (_glProgram->hasUniform(DISTANCE_UNIT_TARGET_NAME))
            {
                _glProgram->bindUniform(DISTANCE_UNIT_TARGET_NAME, mx::Value::createValue(intPortValue));
            }
        }
    }
}

void Material::drawPartition(mx::MeshPartitionPtr part) const
{
    if (!part || !bindPartition(part))
    {
        return;
    }
    mx::MeshIndexBuffer& indexData = part->getIndices();
    glDrawElements(GL_TRIANGLES, (GLsizei) indexData.size(), GL_UNSIGNED_INT, (void*) 0);
    mx::checkGlErrors("after draw partition");
}

void Material::unbindGeometry()
{
    if (_glProgram)
    {
        _glProgram->bind();
        _glProgram->unbindGeometry();
    }
    _boundMesh = nullptr;
}

mx::VariableBlock* Material::getPublicUniforms() const
{
    if (!_hwShader)
    {
        return nullptr;
    }

    mx::ShaderStage& stage = _hwShader->getStage(mx::Stage::PIXEL);
    mx::VariableBlock& block = stage.getUniformBlock(mx::HW::PUBLIC_UNIFORMS);

    return &block;
}

mx::ShaderPort* Material::findUniform(const std::string& path) const
{
    mx::ShaderPort* port = nullptr;
    mx::VariableBlock* publicUniforms = getPublicUniforms();
    if (publicUniforms)
    {
        // Scan block based on path match predicate
        port = publicUniforms->find(
            [path](mx::ShaderPort* port)
            {
                return (port && mx::stringEndsWith(port->getPath(), path));
            });

        // Check if the uniform exists in the shader program
        if (port && !_uniformVariable.count(port->getVariable()))
        {
            port = nullptr;
        }
    }
    return port;
}

void Material::modifyUniform(const std::string& path, mx::ConstValuePtr value, std::string valueString)
{
    mx::ShaderPort* uniform = findUniform(path);
    if (!uniform)
    {
        return;
    }

    _glProgram->bind();
    _glProgram->bindUniform(uniform->getVariable(), value);

    if (valueString.empty())
    {
        valueString = value->getValueString();
    }
    uniform->setValue(mx::Value::createValueFromStrings(valueString, uniform->getType()->getName()));
    if (_doc)
    {
        mx::ElementPtr element = _doc->getDescendant(uniform->getPath());
        if (element)
        {
            mx::ValueElementPtr valueElement = element->asA<mx::ValueElement>();
            if (valueElement)
            {
                valueElement->setValueString(valueString);
            }
        }
    }
}
