#include <MaterialXView/Material.h>

#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXRender/Util.h>

#include <MaterialXFormat/File.h>

#include <nanogui/messagedialog.h>

#include <iostream>

using MatrixXfProxy = Eigen::Map<const ng::MatrixXf>;
using MatrixXuProxy = Eigen::Map<const ng::MatrixXu>;

//
// Material methods
//

bool Material::generateConstantShader(mx::GenContext& context,
                                      mx::DocumentPtr stdLib,
                                      const std::string& shaderName,
                                      const mx::Color3& color)
{
    _hwShader = createConstantShader(context, stdLib, shaderName, color);
    if (!_hwShader)
    {
        return false;
    }
    std::string vertexShader = _hwShader->getSourceCode(mx::Stage::VERTEX);
    std::string pixelShader = _hwShader->getSourceCode(mx::Stage::PIXEL);

    // Compile and return.
    _glShader = std::make_shared<ng::GLShader>();
    return _glShader->init(shaderName, vertexShader, pixelShader);
}

bool Material::generateAmbOccShader(mx::GenContext& context,
                                    const mx::FilePath& filename,
                                    mx::DocumentPtr stdLib,
                                    const mx::FilePath& imagePath)
{
    // Read in the ambient occlusion nodegraph. 
    mx::DocumentPtr doc = mx::createDocument();
    doc->importLibrary(stdLib);
    mx::DocumentPtr envDoc = mx::createDocument();
    mx::readFromXmlFile(envDoc, filename);
    doc->importLibrary(envDoc);

    mx::NodeGraphPtr nodeGraph = doc->getNodeGraph("NG_ambientOcclusion");
    if (!nodeGraph)
    {
        return false;
    }
    mx::NodePtr image = nodeGraph->getNode("image_ao");
    if (!image)
    {
        return false;
    }
    image->setParameterValue("file", imagePath.asString(), mx::FILENAME_TYPE_STRING);
    mx::OutputPtr output = nodeGraph->getOutput("out");
    if (!output)
    {
        return false;
    }

    // Create the shader.
    std::string shaderName = "__AO_SHADER__";
    _hwShader = createShader(shaderName, context, output); 
    if (!_hwShader)
    {
        return false;
    }
    std::string vertexShader = _hwShader->getSourceCode(mx::Stage::VERTEX);
    std::string pixelShader = _hwShader->getSourceCode(mx::Stage::PIXEL);

    // Compile and return.
    _glShader = std::make_shared<ng::GLShader>();
    return _glShader->init(shaderName, vertexShader, pixelShader);
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
    image->setParameterValue("file", imagePath.asString(), mx::FILENAME_TYPE_STRING);
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
    std::string vertexShader = _hwShader->getSourceCode(mx::Stage::VERTEX);
    std::string pixelShader = _hwShader->getSourceCode(mx::Stage::PIXEL);

    // Compile and return.
    _glShader = std::make_shared<ng::GLShader>();
    return _glShader->init(shaderName, vertexShader, pixelShader);
}

bool Material::loadSource(const mx::FilePath& vertexShaderFile, const mx::FilePath& pixelShaderFile, const std::string& shaderName, bool hasTransparency)
{
    _hasTransparency = hasTransparency;

    if (!_glShader)
    {
        _glShader = std::make_shared<ng::GLShader>();
    }

    std::string vertexShader;
    if (!mx::readFile(vertexShaderFile, vertexShader))
    {
        return false;
    }

    std::string pixelShader;
    if (!mx::readFile(pixelShaderFile, pixelShader))
    {
        return false;
    }

    bool initialized = _glShader->init(shaderName, vertexShader, pixelShader);
    updateUniformsList();

    return initialized;
}

void Material::updateUniformsList()
{
    _uniformNames.clear();

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
            _uniformNames.insert(uniformName);
        }
    }
    delete[] uniformName;
}

bool Material::generateShader(mx::GenContext& context)
{
    if (!_elem)
    {
        return false;
    }
    if (!_hwShader)
    {
        _hwShader = createShader("Shader", context, _elem);
    }
    if (!_hwShader)
    {
        return false;
    }

    _hasTransparency = context.getOptions().hwTransparency;

    if (!_glShader)
    {
        std::string vertexShader = _hwShader->getSourceCode(mx::Stage::VERTEX);
        std::string pixelShader = _hwShader->getSourceCode(mx::Stage::PIXEL);

        _glShader = std::make_shared<ng::GLShader>();
        _glShader->init(_elem->getNamePath(), vertexShader, pixelShader);
        updateUniformsList();
    }
    return true;
}

void Material::bindShader()
{
    if (_glShader)
    {
        _glShader->bind();
    }
}

void Material::bindMesh(const mx::MeshPtr mesh) const
{
    if (!mesh || !_glShader)
    {
        return;
    }

    _glShader->bind();
    if (_glShader->attrib("i_position") != -1)
    {
        mx::MeshStreamPtr stream = mesh->getStream(mx::MeshStream::POSITION_ATTRIBUTE, 0);
        mx::MeshFloatBuffer &buffer = stream->getData();
        MatrixXfProxy positions(&buffer[0], stream->getStride(), buffer.size() / stream->getStride());
        _glShader->uploadAttrib("i_position", positions);
    }
    if (_glShader->attrib("i_normal", false) != -1)
    {
        mx::MeshStreamPtr stream = mesh->getStream(mx::MeshStream::NORMAL_ATTRIBUTE, 0);
        mx::MeshFloatBuffer &buffer = stream->getData();
        MatrixXfProxy normals(&buffer[0], stream->getStride(), buffer.size() / stream->getStride());
        _glShader->uploadAttrib("i_normal", normals);
    }
    if (_glShader->attrib("i_tangent", false) != -1)
    {
        mx::MeshStreamPtr stream = mesh->getStream(mx::MeshStream::TANGENT_ATTRIBUTE, 0);
        mx::MeshFloatBuffer &buffer = stream->getData();
        MatrixXfProxy tangents(&buffer[0], stream->getStride(), buffer.size() / stream->getStride());
        _glShader->uploadAttrib("i_tangent", tangents);
    }
    if (_glShader->attrib("i_texcoord_0", false) != -1)
    {
        mx::MeshStreamPtr stream = mesh->getStream(mx::MeshStream::TEXCOORD_ATTRIBUTE, 0);
        mx::MeshFloatBuffer &buffer = stream->getData();
        MatrixXfProxy texcoords(&buffer[0], stream->getStride(), buffer.size() / stream->getStride());
        _glShader->uploadAttrib("i_texcoord_0", texcoords);
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

    mx::Matrix44 viewProj = proj * view;
    mx::Matrix44 invView = view.getInverse();
    mx::Matrix44 invTransWorld = world.getInverse().getTranspose();

    // Bind view properties.
    _glShader->setUniform("u_worldMatrix", ng::Matrix4f(world.getTranspose().data()));
    _glShader->setUniform("u_viewProjectionMatrix", ng::Matrix4f(viewProj.getTranspose().data()));
    if (_glShader->uniform("u_worldInverseTransposeMatrix", false) != -1)
    {
        _glShader->setUniform("u_worldInverseTransposeMatrix", ng::Matrix4f(invTransWorld.getTranspose().data()));
    }
    if (_glShader->uniform("u_viewPosition", false) != -1)
    {
        mx::Vector3 viewPosition(invView[0][3], invView[1][3], invView[2][3]);
        _glShader->setUniform("u_viewPosition", ng::Vector3f(viewPosition.data()));
    }
}

void Material::bindImages(mx::GLTextureHandlerPtr imageHandler, const mx::FileSearchPath& searchPath, const std::string& udim)
{
    if (!_glShader)
    {
        return;
    }

    const std::string IMAGE_SEPARATOR("_");
    const std::string UADDRESS_MODE_POST_FIX("_uaddressmode");
    const std::string VADDRESS_MODE_POST_FIX("_vaddressmode");
    const std::string FILTER_TYPE_POST_FIX("_filtertype");
    const std::string DEFAULT_COLOR_POST_FIX("_default");
    const int INVALID_MAPPED_INT_VALUE = -1; // Any value < 0 is not considered to be invalid

    const mx::VariableBlock* publicUniforms = getPublicUniforms();
    mx::Color4 fallbackColor(0, 0, 0, 1);
    for (auto uniform : publicUniforms->getVariableOrder())
    {
        if (uniform->getType() != MaterialX::Type::FILENAME)
        {
            continue;
        }
        const std::string& uniformName = uniform->getName();
        std::string filename;
        if (uniform->getValue())
        {
            filename = searchPath.find(uniform->getValue()->getValueString());
        }

        // Extract out sampling properties
        mx::ImageSamplingProperties samplingProperties;

        // Get the additional texture parameters based on image uniform name
        // excluding the trailing "_file" postfix string
        std::string root = uniformName;
        size_t pos = root.find_last_of(IMAGE_SEPARATOR);
        if (pos != std::string::npos)
        {
            root = root.substr(0, pos);
        }

        const std::string uaddressmodeStr = root + UADDRESS_MODE_POST_FIX;
        const mx::ShaderPort* port = publicUniforms->find(uaddressmodeStr);
        mx::ValuePtr intValue = port ? port->getValue() : nullptr;
        samplingProperties.uaddressMode = intValue && intValue->isA<int>() ? intValue->asA<int>() : INVALID_MAPPED_INT_VALUE;

        const std::string vaddressmodeStr = root + VADDRESS_MODE_POST_FIX;
        port = publicUniforms->find(vaddressmodeStr);
        intValue = port ? port->getValue() : nullptr;
        samplingProperties.vaddressMode = intValue && intValue->isA<int>() ? intValue->asA<int>() : INVALID_MAPPED_INT_VALUE;

        const std::string filtertypeStr = root + FILTER_TYPE_POST_FIX;
        port = publicUniforms->find(filtertypeStr);
        intValue = port ? port->getValue() : nullptr;
        samplingProperties.filterType = intValue && intValue->isA<int>() ? intValue->asA<int>() : INVALID_MAPPED_INT_VALUE;

        const std::string defaultColorStr = root + DEFAULT_COLOR_POST_FIX;
        port = publicUniforms->find(defaultColorStr);
        mx::ValuePtr colorValue = port ? port->getValue() : nullptr;
        mx::Color4 defaultColor;
        if (colorValue)
        {
            mx::mapValueToColor(colorValue, defaultColor);
            samplingProperties.defaultColor[0] = defaultColor[0];
            samplingProperties.defaultColor[1] = defaultColor[1];
            samplingProperties.defaultColor[2] = defaultColor[2];
            samplingProperties.defaultColor[3] = defaultColor[3];
        }

        mx::ImageDesc desc;
        bindImage(filename, uniformName, imageHandler, desc, samplingProperties, udim, &fallbackColor);
    }
}

bool Material::bindImage(const mx::FilePath& filename, const std::string& uniformName, mx::GLTextureHandlerPtr imageHandler,
                         mx::ImageDesc& desc, const mx::ImageSamplingProperties& samplingProperties, const std::string& udim, mx::Color4* fallbackColor)
{
    if (!_glShader)
    {
        return false;
    }

    // Apply udim string if specified.
    mx::FilePath resolvedFilename;
    if (!udim.empty())
    {
        mx::StringMap map;
        map[mx::UDIM_TOKEN] = udim;
        resolvedFilename = mx::FilePath(mx::replaceSubstrings(filename.asString(), map));
    }
    else
    {
        resolvedFilename = filename;
    }

    // Acquire the given image.
    resolvedFilename = imageHandler->getSearchPath().find(resolvedFilename);
    if (!imageHandler->acquireImage(resolvedFilename, desc, true, fallbackColor) && !filename.isEmpty())
    {
        std::cerr << "Failed to load image: " << resolvedFilename.asString() << std::endl;
    }

    // Bind the image and set its sampling properties.
    int textureLocation = imageHandler->getBoundTextureLocation(desc.resourceId);
    if (textureLocation < 0)
        return false;

    _glShader->setUniform(uniformName, textureLocation, false);
    imageHandler->bindImage(resolvedFilename, samplingProperties);

    return true;
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

void Material::bindLights(mx::LightHandlerPtr lightHandler, mx::GLTextureHandlerPtr imageHandler,
                          const mx::FileSearchPath& imagePath, bool directLighting,
                          bool indirectLighting, mx::HwSpecularEnvironmentMethod specularEnvironmentMethod, int envSamples)
{
    if (!_glShader)
    {
        return;
    }

    _glShader->bind();

    // Bind environment light uniforms and images.
    if (specularEnvironmentMethod == mx::SPECULAR_ENVIRONMENT_FIS)
    {
        if (_glShader->uniform("u_envSamples", false) != -1)
        {
            _glShader->setUniform("u_envSamples", envSamples);
        }
    }
    mx::StringMap lightTextures = {
        { "u_envRadiance", indirectLighting ? (std::string) lightHandler->getLightEnvRadiancePath() : mx::EMPTY_STRING },
        { "u_envIrradiance", indirectLighting ? (std::string) lightHandler->getLightEnvIrradiancePath() : mx::EMPTY_STRING }
    };
    const std::string udim;
    mx::Color4 fallbackColor(0, 0, 0, 1);
    for (auto pair : lightTextures)
    {
        if (_glShader->uniform(pair.first, false) != -1)
        {
            mx::FilePath path = imagePath.find(pair.second);
            const std::string filename = path.asString();

            mx::ImageDesc desc;
            mx::ImageSamplingProperties samplingProperties;
            samplingProperties.uaddressMode = 1;
            samplingProperties.vaddressMode = 1;
            samplingProperties.filterType = 2;

            if (bindImage(filename, pair.first, imageHandler, desc, samplingProperties, udim, &fallbackColor))
            {
                if (specularEnvironmentMethod == mx::SPECULAR_ENVIRONMENT_FIS)
                {
                    // Bind any associated uniforms.
                    if (pair.first == "u_envRadiance")
                    {
                        if (_glShader->uniform("u_envRadianceMips", false) != -1)
                        {
                            _glShader->setUniform("u_envRadianceMips", desc.mipCount);
                        }
                    }
                }
            }
        }
    }

    // Skip direct lights if unsupported by the shader.
    if (_glShader->uniform("u_numActiveLightSources", false) == -1)
    {
        return;
    }

    // Bind direct light sources.
    int lightCount = directLighting ? (int) lightHandler->getLightSources().size() : 0;
    _glShader->setUniform("u_numActiveLightSources", lightCount);
    std::unordered_map<std::string, unsigned int> ids;
    lightHandler->mapNodeDefToIdentiers(lightHandler->getLightSources(), ids);
    size_t index = 0;
    for (mx::NodePtr light : lightHandler->getLightSources())
    {
        auto nodeDef = light->getNodeDef();
        if (!nodeDef)
        {
            continue;
        }
        const std::string prefix = "u_lightData[" + std::to_string(index) + "]";

        // Set light type id
        std::string lightType(prefix + ".type");
        if (_glShader->uniform(lightType, false) != -1)
        {
            unsigned int lightTypeValue = ids[nodeDef->getName()];
            _glShader->setUniform(lightType, lightTypeValue);
        }

        // Set all inputs
        for (auto input : light->getInputs())
        {
            // Make sure we have a value to set
            if (input->hasValue())
            {
                std::string inputName(prefix + "." + input->getName());
                if (_glShader->uniform(inputName, false) != -1)
                {
                    bindUniform(inputName, input->getValue());
                }
            }
        }

        // Set all parameters. Note that upstream node connections are not currently supported.
        for (mx::ParameterPtr param : light->getParameters())
        {
            // Make sure we have a value to set
            if (param->hasValue())
            {
                std::string paramName(prefix + "." + param->getName());
                if (_glShader->uniform(paramName, false) != -1)
                {
                    bindUniform(paramName, param->getValue());
                }
            }
        }

        ++index;
    }
}

void Material::drawPartition(mx::MeshPartitionPtr part) const
{
    if (!bindPartition(part))
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
                return (port && (port->getPath() == path));
            });

        // Check if the uniform exists in the shader program
        if (port && !_uniformNames.count(port->getName()))
        {
            port = nullptr;
        }
    }
    return port;
}
