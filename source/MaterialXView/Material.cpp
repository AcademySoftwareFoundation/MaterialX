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

bool stringEndsWith(const std::string& str, std::string const& end)
{
    if (str.length() >= end.length())
    {
        return !str.compare(str.length() - end.length(), end.length(), end);
    }
    else
    {
        return false;
    }
}

//
// Material methods
//

size_t Material::loadDocument(mx::DocumentPtr destinationDoc, const mx::FilePath& filePath, 
                              mx::DocumentPtr libraries, const DocumentModifiers& modifiers,
                              std::vector<MaterialPtr>& materials)
{
    // Load the content document.
    mx::DocumentPtr doc = mx::createDocument();
    mx::XmlReadOptions readOptions;
    readOptions.readXIncludeFunction = [](mx::DocumentPtr doc, const std::string& filename, const std::string& searchPath, const mx::XmlReadOptions* options)
    {
        mx::FileSearchPath fileSearchPath = mx::FileSearchPath(searchPath);
        fileSearchPath.append(mx::getEnvironmentPath());
        
        mx::FilePath resolvedFilename = fileSearchPath.find(filename);
        if (resolvedFilename.exists())
        {
            readFromXmlFile(doc, resolvedFilename, mx::EMPTY_STRING, options);
        }
        else
        {
            new ng::MessageDialog(nullptr, ng::MessageDialog::Type::Warning, "Include file not found:", filename);
        }
    };
    mx::readFromXmlFile(doc, filePath, mx::EMPTY_STRING, &readOptions);

    // Apply modifiers to the content document if requested.
    for (mx::ElementPtr elem : doc->traverseTree())
    {
        if (modifiers.remapElements.count(elem->getCategory()))
        {
            elem->setCategory(modifiers.remapElements.at(elem->getCategory()));
        }
        if (modifiers.remapElements.count(elem->getName()))
        {
            elem->setName(modifiers.remapElements.at(elem->getName()));
        }
        mx::StringVec attrNames = elem->getAttributeNames();
        for (const std::string& attrName : attrNames)
        {
            if (modifiers.remapElements.count(elem->getAttribute(attrName)))
            {
                elem->setAttribute(attrName, modifiers.remapElements.at(elem->getAttribute(attrName)));
            }
        }
        if (elem->hasFilePrefix() && !modifiers.filePrefixTerminator.empty())
        {
            std::string filePrefix = elem->getFilePrefix();
            if (!stringEndsWith(filePrefix, modifiers.filePrefixTerminator))
            {
                elem->setFilePrefix(filePrefix + modifiers.filePrefixTerminator);
            }
        }
        std::vector<mx::ElementPtr> children = elem->getChildren();
        for (mx::ElementPtr child : children)
        {
            if (modifiers.skipElements.count(child->getCategory()) ||
                modifiers.skipElements.count(child->getName()))
            {
                elem->removeChild(child->getName());
            }
        }
    }

    // Import the given libraries.
    mx::CopyOptions copyOptions;
    copyOptions.skipDuplicateElements = true;
    doc->importLibrary(libraries, &copyOptions);

    // Remap references to unimplemented shader nodedefs.
    for (mx::MaterialPtr material : doc->getMaterials())
    {
        for (mx::ShaderRefPtr shaderRef : material->getShaderRefs())
        {
            mx::NodeDefPtr nodeDef = shaderRef->getNodeDef();
            if (nodeDef && !nodeDef->getImplementation())
            {
                std::vector<mx::NodeDefPtr> altNodeDefs = doc->getMatchingNodeDefs(nodeDef->getNodeString());
                for (mx::NodeDefPtr altNodeDef : altNodeDefs)
                {
                    if (altNodeDef->getImplementation())
                    {
                        shaderRef->setNodeDefString(altNodeDef->getName());
                    }
                }
            }
        }
    }

    // Find new renderable elements.
    size_t previousMaterialCount = materials.size();
    mx::StringVec renderablePaths;
    std::vector<mx::TypedElementPtr> elems;
    mx::findRenderableElements(doc, elems);
    for (mx::TypedElementPtr elem : elems)
    {
        std::string namePath = elem->getNamePath();
        // Skip adding if renderable already exists
        if (!destinationDoc->getDescendant(namePath))
        {
            renderablePaths.push_back(namePath);
        }
    }

    // Check for any udim set.
    mx::ValuePtr udimSetValue = doc->getGeomAttrValue("udimset");

    // Merge the content document into the destination document.
    destinationDoc->importLibrary(doc, &copyOptions);

    // Create new materials.
    for (auto renderablePath : renderablePaths)
    {
        auto elem = destinationDoc->getDescendant(renderablePath)->asA<mx::TypedElement>();
        if (!elem)
        {
            continue;
        }
        if (udimSetValue && udimSetValue->isA<mx::StringVec>())
        {
            for (const std::string& udim : udimSetValue->asA<mx::StringVec>())
            {
                MaterialPtr mat = Material::create();
                mat->setElement(elem);
                mat->setUdim(udim);
                materials.push_back(mat);
            }
        }
        else
        {
            MaterialPtr mat = Material::create();
            mat->setElement(elem);
            materials.push_back(mat);
        }
    }

    return (materials.size() - previousMaterialCount);
}

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

void Material::bindShader(mx::GenContext& context)
{
    generateShader(context);
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

    const mx::VariableBlock* publicUniforms = getPublicUniforms();
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

        mx::ImageDesc desc;
        bindImage(filename, uniformName, imageHandler, desc, udim, nullptr);
    }
}

bool Material::bindImage(std::string filename, const std::string& uniformName, mx::GLTextureHandlerPtr imageHandler,
                         mx::ImageDesc& desc, const std::string& udim, mx::Color4* fallbackColor)
{
    if (!_glShader)
    {
        return false;
    }

    if (filename.empty())
    {
        return false;
    }

    // Apply udim string if specified.
    if (!udim.empty())
    {
        mx::StringMap map;
        map[mx::UDIM_TOKEN] = udim;
        filename = mx::replaceSubstrings(filename, map);
    }

    // Acquire the given image.
    if (!imageHandler->acquireImage(filename, desc, true, fallbackColor))
    {
        std::cerr << "Failed to load image: " << filename << std::endl;
        return false;
    }

    // Bind the image and set its sampling properties.
    _glShader->setUniform(uniformName, desc.resourceId);
    mx::ImageSamplingProperties samplingProperties;
    imageHandler->bindImage(filename, samplingProperties);

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
                          const mx::FileSearchPath& imagePath, int envSamples, bool directLighting, 
                          bool indirectLighting)
{
    if (!_glShader)
    {
        return;
    }

    _glShader->bind();

    // Bind environment light uniforms and images.
    if (_glShader->uniform("u_envSamples", false) != -1)
    {
        _glShader->setUniform("u_envSamples", envSamples);
    }
    mx::StringMap lightTextures = {
        { "u_envRadiance", indirectLighting ? (std::string) lightHandler->getLightEnvRadiancePath() : mx::EMPTY_STRING },
        { "u_envIrradiance", indirectLighting ? (std::string) lightHandler->getLightEnvIrradiancePath() : mx::EMPTY_STRING }
    };
    const std::string udim;
    mx::Color4 fallbackColor = { 0.0, 0.0, 0.0, 1.0 };
    for (auto pair : lightTextures)
    {
        if (_glShader->uniform(pair.first, false) != -1)
        {
            // Access cached image or load from disk.
            mx::FilePath path = imagePath.find(pair.second);
            const std::string filename = path.asString();

            mx::ImageDesc desc;
            if (bindImage(filename, pair.first, imageHandler, desc, udim, &fallbackColor))
            {
                // Bind any associated uniforms.
                if (pair.first == "u_envRadiance")
                {
                    _glShader->setUniform("u_envRadianceMips", desc.mipCount);
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
