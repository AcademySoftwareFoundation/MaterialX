#include <MaterialXView/Material.h>

#include <MaterialXRenderGlsl/GLTextureHandler.h>

#include <MaterialXRender/Util.h>

#include <MaterialXGenShader/Shader.h>

#include <MaterialXFormat/Util.h>

#include <nanogui/messagedialog.h>

#include <iostream>

namespace {

using MatrixXfProxy = Eigen::Map<const ng::MatrixXf>;
using MatrixXuProxy = Eigen::Map<const ng::MatrixXu>;

const mx::Color4 IMAGE_DEFAULT_COLOR(0, 0, 0, 1);

const float PI = std::acos(-1.0f);

} // anonymous namespace

//
// Material methods
//

bool Material::loadSource(const mx::FilePath& vertexShaderFile, const mx::FilePath& pixelShaderFile, const std::string& shaderName, bool hasTransparency)
{
    _hasTransparency = hasTransparency;

    if (!_glShader)
    {
        _glShader = std::make_shared<ng::GLShader>();
    }

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

    bool initialized = _glShader->init(shaderName, vertexShader, pixelShader);
    updateUniformsList();

    return initialized;
}

void Material::updateUniformsList()
{
    _uniformVariable.clear();
    if (!_glShader)
    {
        return;
    }

    // Must bind to be able to inspect the uniforms
    _glShader->bind();

    int _programId = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &_programId);
    int uniformCount = -1;
    int uniformSize = -1;
    GLenum uniformType = 0;
    int maxNameLength = 0;
    glGetProgramiv(_programId, GL_ACTIVE_UNIFORMS, &uniformCount);
    glGetProgramiv(_programId, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);
    char* uniformName = new char[maxNameLength];
    for (int i = 0; i < uniformCount; i++)
    {
        glGetActiveUniform(_programId, GLuint(i), maxNameLength, nullptr, &uniformSize, &uniformType, uniformName);
        GLint uniformLocation = glGetUniformLocation(_programId, uniformName);
        if (uniformLocation >= 0)
        {
            _uniformVariable.insert(uniformName);
        }
    }
    delete[] uniformName;
}

void Material::clearShader()
{
    _hwShader = nullptr;
    _glShader = nullptr;
    _uniformVariable.clear();
}

bool Material::generateShader(mx::GenContext& context)
{
    if (!_elem)
    {
        return false;
    }

    _hasTransparency = context.getOptions().hwTransparency &&
                       mx::isTransparentSurface(_elem, context.getShaderGenerator());

    mx::GenContext materialContext = context;
    materialContext.getOptions().hwTransparency = _hasTransparency;
    materialContext.getOptions().hwShadowMap = materialContext.getOptions().hwShadowMap && !_hasTransparency;

    // Initialize in case creation fails and throws an exception
    clearShader();

    _hwShader = createShader("Shader", materialContext, _elem);
    if (!_hwShader)
    {
        return false;
    }

    std::string vertexShader = _hwShader->getSourceCode(mx::Stage::VERTEX);
    std::string pixelShader = _hwShader->getSourceCode(mx::Stage::PIXEL);

    _glShader = std::make_shared<ng::GLShader>();
    _glShader->init(_elem->getNamePath(), vertexShader, pixelShader);

    updateUniformsList();

    return true;
}

bool Material::generateShader(mx::ShaderPtr hwShader)
{
    _hwShader = hwShader;

    std::string vertexShader = _hwShader->getSourceCode(mx::Stage::VERTEX);
    std::string pixelShader = _hwShader->getSourceCode(mx::Stage::PIXEL);

    _glShader = std::make_shared<ng::GLShader>();
    _glShader->init(hwShader->getName(), vertexShader, pixelShader);

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
    if (_glShader)
    {
        _glShader->bind();
    }
}

void Material::bindMesh(mx::MeshPtr mesh) const
{
    if (!mesh || !_glShader)
    {
        return;
    }

    _glShader->bind();
    if (_glShader->attrib(mx::HW::IN_POSITION) != -1)
    {
        mx::MeshStreamPtr stream = mesh->getStream(mx::MeshStream::POSITION_ATTRIBUTE, 0);
        mx::MeshFloatBuffer &buffer = stream->getData();
        MatrixXfProxy positions(&buffer[0], stream->getStride(), buffer.size() / stream->getStride());
        _glShader->uploadAttrib(mx::HW::IN_POSITION, positions);
    }
    if (_glShader->attrib(mx::HW::IN_NORMAL, false) != -1)
    {
        mx::MeshStreamPtr stream = mesh->getStream(mx::MeshStream::NORMAL_ATTRIBUTE, 0);
        mx::MeshFloatBuffer &buffer = stream->getData();
        MatrixXfProxy normals(&buffer[0], stream->getStride(), buffer.size() / stream->getStride());
        _glShader->uploadAttrib(mx::HW::IN_NORMAL, normals);
    }
    if (_glShader->attrib(mx::HW::IN_TANGENT, false) != -1)
    {
        mx::MeshStreamPtr stream = mesh->getStream(mx::MeshStream::TANGENT_ATTRIBUTE, 0);
        mx::MeshFloatBuffer &buffer = stream->getData();
        MatrixXfProxy tangents(&buffer[0], stream->getStride(), buffer.size() / stream->getStride());
        _glShader->uploadAttrib(mx::HW::IN_TANGENT, tangents);
    }
    const std::string texcoord = mx::HW::IN_TEXCOORD + "_0";
    if (_glShader->attrib(texcoord, false) != -1)
    {
        mx::MeshStreamPtr stream = mesh->getStream(mx::MeshStream::TEXCOORD_ATTRIBUTE, 0);
        mx::MeshFloatBuffer &buffer = stream->getData();
        MatrixXfProxy texcoords(&buffer[0], stream->getStride(), buffer.size() / stream->getStride());
        _glShader->uploadAttrib(texcoord, texcoords);
    }
}

bool Material::bindPartition(mx::MeshPartitionPtr part) const
{
    if (!_glShader)
    {
        return false;
    }

    _glShader->bind();
    MatrixXuProxy indices(&part->getIndices()[0], 3, part->getIndices().size() / 3);
    _glShader->uploadIndices(indices);

    return true;
}

void Material::bindViewInformation(const mx::Matrix44& world, const mx::Matrix44& view, const mx::Matrix44& proj)
{
    if (!_glShader)
    {
        return;
    }

    mx::Matrix44 viewProj = view * proj;
    mx::Matrix44 invView = view.getInverse();
    mx::Matrix44 invTransWorld = world.getInverse().getTranspose();
    mx::Vector3 viewPosition(invView[3][0], invView[3][1], invView[3][2]);

    // Bind view properties.
    _glShader->setUniform(mx::HW::WORLD_MATRIX, ng::Matrix4f(world.data()), false);
    _glShader->setUniform(mx::HW::VIEW_PROJECTION_MATRIX, ng::Matrix4f(viewProj.data()), false);
    _glShader->setUniform(mx::HW::WORLD_INVERSE_TRANSPOSE_MATRIX, ng::Matrix4f(invTransWorld.data()), false);
    _glShader->setUniform(mx::HW::VIEW_POSITION, ng::Vector3f(viewPosition.data()), false);
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
    if (!_glShader)
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

        mx::ImagePtr image = bindImage(filename, uniformVariable, imageHandler, samplingProperties, &IMAGE_DEFAULT_COLOR);
        if (image)
        {
            _boundImages.push_back(image);
        }
    }
}

mx::ImagePtr Material::bindImage(const mx::FilePath& filePath, const std::string& uniformName, mx::ImageHandlerPtr imageHandler,
                                 const mx::ImageSamplingProperties& samplingProperties, const mx::Color4* fallbackColor)
{
    if (!_glShader)
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
    mx::ImagePtr image = imageHandler->acquireImage(filePath, true, fallbackColor);
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
            _glShader->setUniform(uniformName, textureLocation, false);
            return image;
        }
    }
    return nullptr;
}

void Material::bindUniform(const std::string& name, mx::ConstValuePtr value)
{
    if (!value)
    {
        return;
    }

    if (value->isA<float>())
    {
        float v = value->asA<float>();
        _glShader->setUniform(name, v);
    }
    else if (value->isA<int>())
    {
        int v = value->asA<int>();
        _glShader->setUniform(name, v);
    }
    else if (value->isA<bool>())
    {
        bool v = value->asA<bool>();
        _glShader->setUniform(name, v);
    }
    else if (value->isA<mx::Color2>())
    {
        mx::Color2 v = value->asA<mx::Color2>();
        _glShader->setUniform(name, ng::Vector2f(v.data()));
    }
    else if (value->isA<mx::Color3>())
    {
        mx::Color3 v = value->asA<mx::Color3>();
        _glShader->setUniform(name, ng::Vector3f(v.data()));
    }
    else if (value->isA<mx::Color4>())
    {
        mx::Color4 v = value->asA<mx::Color4>();
        _glShader->setUniform(name, ng::Vector4f(v.data()));
    }
    else if (value->isA<mx::Vector2>())
    {
        mx::Vector2 v = value->asA<mx::Vector2>();
        _glShader->setUniform(name, ng::Vector2f(v.data()));
    }
    else if (value->isA<mx::Vector3>())
    {
        mx::Vector3 v = value->asA<mx::Vector3>();
        _glShader->setUniform(name, ng::Vector3f(v.data()));
    }
    else if (value->isA<mx::Vector4>())
    {
        mx::Vector4 v = value->asA<mx::Vector4>();
        _glShader->setUniform(name, ng::Vector4f(v.data()));
    }
}

void Material::bindLights(const mx::GenContext& genContext, mx::LightHandlerPtr lightHandler, mx::ImageHandlerPtr imageHandler,
                          const LightingState& lightingState, const ShadowState& shadowState)
{
    if (!_glShader)
    {
        return;
    }

    _glShader->bind();


    // Bind environment lighting properties.
    if (_glShader->uniform(mx::HW::ENV_MATRIX, false) != -1)
    {
        mx::Matrix44 envRotation = mx::Matrix44::createRotationY(PI) * lightingState.lightTransform.getTranspose();
        _glShader->setUniform(mx::HW::ENV_MATRIX, ng::Matrix4f(envRotation.data()));
    }
    if (_glShader->uniform(mx::HW::ENV_RADIANCE_SAMPLES, false) != -1)
    {
        _glShader->setUniform(mx::HW::ENV_RADIANCE_SAMPLES, lightingState.envSamples);
    }
    mx::ImageMap envImages =
    {
        { mx::HW::ENV_RADIANCE, lightingState.indirectLighting ? lightHandler->getEnvRadianceMap() : imageHandler->getZeroImage() },
        { mx::HW::ENV_IRRADIANCE, lightingState.indirectLighting ? lightHandler->getEnvIrradianceMap() : imageHandler->getZeroImage() }
    };
    for (const auto& env : envImages)
    {
        std::string uniform = env.first;
        mx::ImagePtr image = env.second;
        if (image && _glShader->uniform(env.first, false) != -1)
        {
            mx::ImageSamplingProperties samplingProperties;
            samplingProperties.uaddressMode = mx::ImageSamplingProperties::AddressMode::PERIODIC;
            samplingProperties.vaddressMode = mx::ImageSamplingProperties::AddressMode::CLAMP;
            samplingProperties.filterType = mx::ImageSamplingProperties::FilterType::LINEAR;

            // Bind the environment image.
            if (imageHandler->bindImage(image, samplingProperties))
            {
                mx::GLTextureHandlerPtr textureHandler = std::static_pointer_cast<mx::GLTextureHandler>(imageHandler);
                int textureLocation = textureHandler->getBoundTextureLocation(image->getResourceId());
                if (textureLocation >= 0)
                {
                    _glShader->setUniform(uniform, textureLocation, false);
                }

                // Bind any associated uniforms.
                if (genContext.getOptions().hwSpecularEnvironmentMethod == mx::SPECULAR_ENVIRONMENT_FIS)
                {
                    if (uniform == mx::HW::ENV_RADIANCE)
                    {
                        if (_glShader->uniform(mx::HW::ENV_RADIANCE_MIPS, false) != -1)
                        {
                            _glShader->setUniform(mx::HW::ENV_RADIANCE_MIPS, image->getMaxMipCount());
                        }
                    }
                }
            }
        }
    }

    // Bind direct lighting properties.
    if (_glShader->uniform(mx::HW::NUM_ACTIVE_LIGHT_SOURCES, false) != -1)
    {
        int lightCount = lightingState.directLighting ? (int) lightHandler->getLightSources().size() : 0;
        _glShader->setUniform(mx::HW::NUM_ACTIVE_LIGHT_SOURCES, lightCount);
        mx::LightIdMap idMap = lightHandler->computeLightIdMap(lightHandler->getLightSources());
        size_t index = 0;
        for (mx::NodePtr light : lightHandler->getLightSources())
        {
            auto nodeDef = light->getNodeDef();
            if (!nodeDef)
            {
                continue;
            }

            const std::string prefix = mx::HW::LIGHT_DATA_INSTANCE + "[" + std::to_string(index) + "]";

            // Set light type id
            std::string lightType(prefix + ".type");
            if (_glShader->uniform(lightType, false) != -1)
            {
                unsigned int lightTypeValue = idMap[nodeDef->getName()];
                _glShader->setUniform(lightType, lightTypeValue);
            }

            // Set all inputs
            for (const auto& input : light->getInputs())
            {
                // Make sure we have a value to set
                if (input->hasValue())
                {
                    std::string inputName(prefix + "." + input->getName());
                    if (_glShader->uniform(inputName, false) != -1)
                    {
                        if (input->getName() == "direction" && input->hasValue() && input->getValue()->isA<mx::Vector3>())
                        {
                            mx::Vector3 dir = input->getValue()->asA<mx::Vector3>();
                            dir = lightingState.lightTransform.transformVector(dir);
                            bindUniform(inputName, mx::Value::createValue(dir));
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

    // Bind shadow map properties
    if (shadowState.shadowMap && _glShader->uniform(mx::HW::SHADOW_MAP, false) != -1)
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
                _glShader->setUniform(mx::HW::SHADOW_MAP, textureLocation);
            }
        }
        _glShader->setUniform(mx::HW::SHADOW_MATRIX, ng::Matrix4f(shadowState.shadowMatrix.data()), false);
    }

    // Bind ambient occlusion properties.
    if (shadowState.ambientOcclusionMap && _glShader->uniform(mx::HW::AMB_OCC_MAP, false) != -1)
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
                _glShader->setUniform(mx::HW::AMB_OCC_MAP, textureLocation);
            }
        }
        _glShader->setUniform(mx::HW::AMB_OCC_GAIN, shadowState.ambientOcclusionGain, false);
    }

    // Bind the directional albedo table.
    if (genContext.getOptions().hwDirectionalAlbedoMethod == mx::DIRECTIONAL_ALBEDO_TABLE)
    {
        if (_glShader->uniform(mx::HW::ALBEDO_TABLE, false) != -1)
        {
            mx::ImageSamplingProperties samplingProperties;
            samplingProperties.uaddressMode = mx::ImageSamplingProperties::AddressMode::CLAMP;
            samplingProperties.vaddressMode = mx::ImageSamplingProperties::AddressMode::CLAMP;
            samplingProperties.filterType = mx::ImageSamplingProperties::FilterType::LINEAR;
            mx::ImagePtr albedoTable = lightHandler->getAlbedoTable();
            if (imageHandler->bindImage(albedoTable, samplingProperties))
            {
                mx::GLTextureHandlerPtr textureHandler = std::static_pointer_cast<mx::GLTextureHandler>(imageHandler);
                int textureLocation = textureHandler->getBoundTextureLocation(albedoTable->getResourceId());
                if (textureLocation >= 0)
                {
                    _glShader->setUniform(mx::HW::ALBEDO_TABLE, textureLocation, false);
                }
            }
        }
    }
}

void Material::bindUnits(mx::UnitConverterRegistryPtr& registry, const mx::GenContext& context)
{
    static std::string DISTANCE_UNIT_TARGET_NAME = "u_distanceUnitTarget";

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
            _glShader->bind();
            if (_glShader->uniform(DISTANCE_UNIT_TARGET_NAME, false) != -1)
            {
                _glShader->setUniform(DISTANCE_UNIT_TARGET_NAME, intPortValue);
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
    _glShader->drawIndexed(GL_TRIANGLES, 0, (uint32_t) part->getFaceCount());
}

mx::VariableBlock* Material::getPublicUniforms() const
{
    if (!_hwShader)
    {
        return nullptr;
    }
    try
    {
        mx::ShaderStage& stage = _hwShader->getStage(mx::Stage::PIXEL);
        mx::VariableBlock& block = stage.getUniformBlock(mx::HW::PUBLIC_UNIFORMS);
        return &block;
    }
    catch (mx::Exception& e)
    {
        new ng::MessageDialog(nullptr, ng::MessageDialog::Type::Warning, "Unable to find shader uniforms", e.what());
    }
    return nullptr;
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

void Material::changeUniformElement(mx::ShaderPort* uniform, const std::string& value)
{
    if (!uniform)
    {
        throw std::runtime_error("Null ShaderPort");
    }
    uniform->setValue(mx::Value::createValueFromStrings(value, uniform->getType()->getName()));
    if (_doc)
    {
        mx::ElementPtr element = _doc->getDescendant(uniform->getPath());
        if (element)
        {
            mx::ValueElementPtr valueElement = element->asA<mx::ValueElement>();
            if (valueElement)
            {
                valueElement->setValueString(value);
            }
        }
    }
}

void Material::setUniformInt(const std::string& path, int value)
{
    mx::ShaderPort* uniform = findUniform(path);
    if (uniform)
    {
        getShader()->bind();
        getShader()->setUniform(uniform->getVariable(), value);
        std::stringstream intValue;
        intValue << value;
        changeUniformElement(uniform, intValue.str());
    }
}

void Material::setUniformFloat(const std::string& path, float value)
{
    mx::ShaderPort* uniform = findUniform(path);
    if (uniform)
    {
        getShader()->bind();
        getShader()->setUniform(uniform->getVariable(), value);
        std::stringstream floatValue;
        floatValue << value;
        changeUniformElement(uniform, floatValue.str());
    }
}

void Material::setUniformVec2(const std::string& path, const ng::Vector2f& value)
{
    mx::ShaderPort* uniform = findUniform(path);
    if (uniform)
    {
        getShader()->bind();
        getShader()->setUniform(uniform->getVariable(), value);
        std::stringstream vec2Value;
        vec2Value << value[0] << mx::ARRAY_VALID_SEPARATORS << value[1];
        changeUniformElement(uniform, vec2Value.str());
    }
}

void Material::setUniformVec3(const std::string& path, const ng::Vector3f& value)
{
    mx::ShaderPort* uniform = findUniform(path);
    if (uniform)
    {        
        getShader()->bind();
        getShader()->setUniform(uniform->getVariable(), value);
        std::stringstream vec3Value;
        vec3Value << value[0] << mx::ARRAY_VALID_SEPARATORS << value[1] << mx::ARRAY_VALID_SEPARATORS << value[2];
        changeUniformElement(uniform, vec3Value.str());
    }    
}

void Material::setUniformVec4(const std::string& path, const ng::Vector4f& value)
{
    mx::ShaderPort* uniform = findUniform(path);
    if (uniform)
    {
        getShader()->bind();
        getShader()->setUniform(uniform->getVariable(), value);
        std::stringstream vec4Value;        
        vec4Value << value[0] << mx::ARRAY_VALID_SEPARATORS << value[1] << mx::ARRAY_VALID_SEPARATORS << value[2] << mx::ARRAY_VALID_SEPARATORS << value[3];
        changeUniformElement(uniform, vec4Value.str());
    }
}

void Material::setUniformEnum(const std::string& path, int index, const std::string& value)
{
    mx::ShaderPort* uniform = findUniform(path);
    if (uniform)
    {
        getShader()->bind();
        getShader()->setUniform(uniform->getVariable(), index);
        changeUniformElement(uniform, value);
    }
}
