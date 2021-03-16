//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRenderGlsl/External/GLew/glew.h>
#include <MaterialXRenderGlsl/GlslProgram.h>
#include <MaterialXRenderGlsl/GLTextureHandler.h>
#include <MaterialXRenderGlsl/GLUtil.h>

#include <MaterialXRender/ShaderRenderer.h>

#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <iostream>

namespace MaterialX
{

// OpenGL Constants
unsigned int GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID = 0;
int GlslProgram::UNDEFINED_OPENGL_PROGRAM_LOCATION = -1;
int GlslProgram::Input::INVALID_OPENGL_TYPE = -1;

//
// GlslProgram methods
//

GlslProgram::GlslProgram() :
    _programId(UNDEFINED_OPENGL_RESOURCE_ID),
    _shader(nullptr),
    _vertexArray(UNDEFINED_OPENGL_RESOURCE_ID),
    _lastGeometryName(EMPTY_STRING)
{
}

GlslProgram::~GlslProgram()
{
    deleteProgram();
}

void GlslProgram::setStages(ShaderPtr shader)
{
    if (!shader)
    {
        StringVec errors;
        throw ExceptionShaderRenderError("Cannot set stages using null hardware shader.", errors);
    }

    // Clear out any old data
    clearStages();

    // Extract out the shader code per stage
    _shader = shader;
    for (size_t i =0; i<shader->numStages(); ++i)
    {
        const ShaderStage& stage = shader->getStage(i);
        addStage(stage.getName(), stage.getSourceCode());
    }

    // A stage change invalidates any cached parsed inputs
    clearInputLists();
}

void GlslProgram::addStage(const string& stage, const string& sourcCode)
{
    _stages[stage] = sourcCode;
}

const string& GlslProgram::getStageSourceCode(const string& stage) const
{
    auto it = _stages.find(stage);
    if (it != _stages.end())
    {
        return it->second;
    }
    return EMPTY_STRING;
}

void GlslProgram::clearStages()
{
    _stages.clear();

    // Clearing stages invalidates any cached inputs
    clearInputLists();
}

void GlslProgram::deleteProgram()
{
    if (_programId > UNDEFINED_OPENGL_RESOURCE_ID)
    {
        glUseProgram(0);
        glDeleteProgram(_programId);
        _programId = UNDEFINED_OPENGL_RESOURCE_ID;
    }

    // Program deleted, so also clear cached input lists
    clearInputLists();
}

unsigned int GlslProgram::build()
{
    StringVec errors;
    const string errorType("GLSL program creation error.");

    deleteProgram();

    GLint GLStatus = GL_FALSE;
    int GLInfoLogLength = 0;

    unsigned int stagesBuilt = 0;
    unsigned int desiredStages = 0;
    for (const auto& it : _stages)
    {
        if (it.second.length())
            desiredStages++;
    }

    // Create vertex shader
    GLuint vertexShaderId = UNDEFINED_OPENGL_RESOURCE_ID;
    string &vertexShaderSource = _stages[Stage::VERTEX];
    if (vertexShaderSource.length())
    {
        vertexShaderId = glCreateShader(GL_VERTEX_SHADER);

        // Compile vertex shader
        const char* vertexChar = vertexShaderSource.c_str();
        glShaderSource(vertexShaderId, 1, &vertexChar, nullptr);
        glCompileShader(vertexShaderId);

        // Check Vertex Shader
        glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &GLStatus);
        if (GLStatus == GL_FALSE)
        {
            errors.push_back("Error in compiling vertex shader:");
            glGetShaderiv(vertexShaderId, GL_INFO_LOG_LENGTH, &GLInfoLogLength);
            if (GLInfoLogLength > 0)
            {
                std::vector<char> vsErrorMessage(GLInfoLogLength + 1);
                glGetShaderInfoLog(vertexShaderId, GLInfoLogLength, nullptr, &vsErrorMessage[0]);
                errors.push_back(&vsErrorMessage[0]);
            }
        }
        else
        {
            stagesBuilt++;
        }
    }

    // Create fragment shader
    GLuint fragmentShaderId = UNDEFINED_OPENGL_RESOURCE_ID;
    string& fragmentShaderSource = _stages[Stage::PIXEL];
    if (fragmentShaderSource.length())
    {
        fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

        // Compile fragment shader
        const char *fragmentChar = fragmentShaderSource.c_str();
        glShaderSource(fragmentShaderId, 1, &fragmentChar, nullptr);
        glCompileShader(fragmentShaderId);

        // Check fragment shader
        glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &GLStatus);
        if (GLStatus == GL_FALSE)
        {
            errors.push_back("Error in compiling fragment shader:");
            glGetShaderiv(fragmentShaderId, GL_INFO_LOG_LENGTH, &GLInfoLogLength);
            if (GLInfoLogLength > 0)
            {
                std::vector<char> fsErrorMessage(GLInfoLogLength + 1);
                glGetShaderInfoLog(fragmentShaderId, GLInfoLogLength, nullptr, &fsErrorMessage[0]);
                errors.push_back(&fsErrorMessage[0]);
            }
        }
        else
        {
            stagesBuilt++;
        }
    }

    // Link stages to a programs
    if (stagesBuilt == desiredStages)
    {
        _programId = glCreateProgram();
        glAttachShader(_programId, vertexShaderId);
        glAttachShader(_programId, fragmentShaderId);
        glLinkProgram(_programId);

        // Check the program
        glGetProgramiv(_programId, GL_LINK_STATUS, &GLStatus);
        if (GLStatus == GL_FALSE)
        {
            errors.push_back("Error in linking program:");
            glGetProgramiv(_programId, GL_INFO_LOG_LENGTH, &GLInfoLogLength);
            if (GLInfoLogLength > 0)
            {
                std::vector<char> ProgramErrorMessage(GLInfoLogLength + 1);
                glGetProgramInfoLog(_programId, GLInfoLogLength, nullptr, &ProgramErrorMessage[0]);
                errors.push_back(&ProgramErrorMessage[0]);
            }
        }
    }
    else
    {
        errors.push_back("Failed to build all stages.");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    // Cleanup
    if (vertexShaderId > UNDEFINED_OPENGL_RESOURCE_ID)
    {
        if (_programId > UNDEFINED_OPENGL_RESOURCE_ID)
        {
            glDetachShader(_programId, vertexShaderId);
        }
        glDeleteShader(vertexShaderId);
    }
    if (fragmentShaderId > UNDEFINED_OPENGL_RESOURCE_ID)
    {
        if (_programId > UNDEFINED_OPENGL_RESOURCE_ID)
        {
            glDetachShader(_programId, fragmentShaderId);
        }
        glDeleteShader(fragmentShaderId);
    }

    // If we encountered any errors while trying to create return list
    // of all errors. That is we collect all errors per stage plus any
    // errors during linking and throw one exception for them all so that
    // if there is a failure a complete set of issues is returned. We do
    // this after cleanup so keep GL state clean.
    if (!errors.empty())
    {
        throw ExceptionShaderRenderError(errorType, errors);
    }

    return _programId;
}

bool GlslProgram::bind()
{
    if (_programId > UNDEFINED_OPENGL_RESOURCE_ID)
    {
        glUseProgram(_programId);
        checkGlErrors("after program bind");
        return true;
    }
    return false;
}

void GlslProgram::bindInputs(ViewHandlerPtr viewHandler,
                             GeometryHandlerPtr geometryHandler,
                             ImageHandlerPtr imageHandler,
                             LightHandlerPtr lightHandler)
{
    // Bind the program to use
    if (!bind())
    {
        const string errorType("GLSL bind inputs error.");
        StringVec errors;
        errors.push_back("Cannot bind inputs without a valid program");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    checkGlErrors("after program bind inputs");

    // Parse for uniforms and attributes
    getUniformsList();
    getAttributesList();

    const MeshList& meshes = geometryHandler->getMeshes();
    if (meshes.size() > 0 && meshes[0]->getIdentifier() != _lastGeometryName)
    {
        unbindGeometry();
        _lastGeometryName = meshes[0]->getIdentifier();
    }
    // Bind based on inputs found
    bindViewInformation(viewHandler);
    for (const auto& mesh : geometryHandler->getMeshes())
    {
        bindMesh(mesh);
    }

    bindTextures(imageHandler);
    bindTimeAndFrame();
    bindLighting(lightHandler, imageHandler);

    // Set up raster state for transparency as needed
    if (_shader->hasAttribute(HW::ATTR_TRANSPARENT))
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }
}

void GlslProgram::unbindInputs(ImageHandlerPtr imageHandler)
{
    imageHandler->unbindImages();

    // Clean up raster state if transparency was set in bindInputs()
    if (_shader->hasAttribute(HW::ATTR_TRANSPARENT))
    {
        glDisable(GL_BLEND);
    }
}

void GlslProgram::bindAttribute(const GlslProgram::InputMap& inputs, MeshPtr mesh)
{
    const string errorType("GLSL bind attribute error.");
    StringVec errors;

    if (!mesh)
    {
        errors.push_back("No geometry set to bind");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    const size_t FLOAT_SIZE = sizeof(float);

    for (const auto& input : inputs)
    {
        int location = input.second->location;
        unsigned int index = input.second->value ? input.second->value->asA<int>() : 0;

        unsigned int stride = 0;
        MeshStreamPtr stream = mesh->getStream(input.first);
        if (!stream)
        {
            errors.push_back("Geometry buffer could not be retrieved for binding: " + input.first + ". Index: " + std::to_string(index));
            throw ExceptionShaderRenderError(errorType, errors);
        }
        MeshFloatBuffer& attributeData = stream->getData();
        stride = stream->getStride();

        if (attributeData.empty() || (stride == 0))
        {
            errors.push_back("Geometry buffer could not be retrieved for binding: " + input.first + ". Index: " + std::to_string(index));
            throw ExceptionShaderRenderError(errorType, errors);
        }

        if (_attributeBufferIds.find(input.first) == _attributeBufferIds.end())
        {
            const float* bufferData = &attributeData[0];
            size_t bufferSize = attributeData.size()*FLOAT_SIZE;

            // Create a buffer based on attribute type.
            unsigned int bufferId = GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
            glGenBuffers(1, &bufferId);
            glBindBuffer(GL_ARRAY_BUFFER, bufferId);
            glBufferData(GL_ARRAY_BUFFER, bufferSize, bufferData, GL_STATIC_DRAW);

            _attributeBufferIds[input.first] = bufferId;
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, _attributeBufferIds[input.first]);
        }

        glEnableVertexAttribArray(location);
        _enabledStreamLocations.insert(location);
        if (input.second->gltype != GL_INT)
        {
            glVertexAttribPointer(location, stride, GL_FLOAT, GL_FALSE, 0, nullptr);
        }
        else
        {
            glVertexAttribIPointer(location, stride, GL_INT, 0, nullptr);
        }
    }
}

void GlslProgram::bindPartition(MeshPartitionPtr part)
{
    StringVec errors;
    const string errorType("GLSL geometry bind error.");
    if (!part || part->getFaceCount() == 0)
    {
        errors.push_back("Cannot bind geometry partition");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    if (_indexBufferIds.find(part) != _indexBufferIds.end())
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferIds[part]);
    }
    else
    {
        MeshIndexBuffer& indexData = part->getIndices();
        size_t indexBufferSize = indexData.size();
        unsigned int indexBuffer = GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize * sizeof(uint32_t), &indexData[0], GL_STATIC_DRAW);
        _indexBufferIds[part] = indexBuffer;
    }
}

void GlslProgram::bindMesh(MeshPtr mesh)
{
    _enabledStreamLocations.clear(); 
    StringVec errors;
    const string errorType("GLSL geometry bind error.");

    if (_programId == UNDEFINED_OPENGL_RESOURCE_ID)
    {
        errors.push_back("Cannot bind geometry without a valid program");
        throw ExceptionShaderRenderError(errorType, errors);
    }
    if (!mesh)
    {
        errors.push_back("No mesh to bind");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    GlslProgram::InputMap foundList;
    const GlslProgram::InputMap& attributeList = getAttributesList();

    if (_vertexArray == UNDEFINED_OPENGL_RESOURCE_ID)
    {
        // Set up vertex arrays
        glGenVertexArrays(1, &_vertexArray);
    }
    glBindVertexArray(_vertexArray);

    // Bind positions
    findInputs(HW::IN_POSITION, attributeList, foundList, true);
    if (foundList.size())
    {
        bindAttribute(foundList, mesh);
    }

    // Bind normals
    findInputs(HW::IN_NORMAL, attributeList, foundList, true);
    if (foundList.size())
    {
        bindAttribute(foundList, mesh);
    }

    // Bind tangents
    findInputs(HW::IN_TANGENT, attributeList, foundList, true);
    if (foundList.size())
    {
        bindAttribute(foundList, mesh);
    }

    // Bind colors
    // Search for anything that starts with the color prefix
    findInputs(HW::IN_COLOR + "_", attributeList, foundList, false);
    if (foundList.size())
    {
        bindAttribute(foundList, mesh);
    }

    // Bind texture coordinates
    // Search for anything that starts with the texcoord prefix
    findInputs(HW::IN_TEXCOORD + "_", attributeList, foundList, false);
    if (foundList.size())
    {
        bindAttribute(foundList, mesh);
    }

    // Bind any named varying geometric property information
    findInputs(HW::IN_GEOMPROP + "_", attributeList, foundList, false);
    if (foundList.size())
    {
        bindAttribute(foundList, mesh);
    }

    // Bind any named uniform geometric property information
    const GlslProgram::InputMap& uniformList = getUniformsList();
    findInputs(HW::GEOMPROP + "_", uniformList, foundList, false);
    for (const auto& input : foundList)
    {
        // Only handle float1-4 types for now
        switch (input.second->gltype)
        {
            case GL_INT:
                glUniform1i(input.second->location, 1);
                break;
            case GL_FLOAT:
                glUniform1f(input.second->location, 0.0f);
                break;
            case GL_FLOAT_VEC2:
                glUniform2f(input.second->location, 0.0f, 0.0f);
                break;
            case GL_FLOAT_VEC3:
                glUniform3f(input.second->location, 0.0f, 0.0f, 0.0f);
                break;
            case GL_FLOAT_VEC4:
                glUniform4f(input.second->location, 0.0f, 0.0f, 0.0f, 1.0f);
                break;
            default:
                break;
        }
    }

    checkGlErrors("after program bind mesh");
}

void GlslProgram::unbindGeometry()
{
    // Cleanup attribute bindings
    //
    int numberAttributes = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numberAttributes);
    for (int i : _enabledStreamLocations)
    {
        glDisableVertexAttribArray(i);
    }
    _enabledStreamLocations.clear();
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);

    // Clean up buffers
    //
    for (const auto& attributeBufferId : _attributeBufferIds)
    {
        unsigned int bufferId = attributeBufferId.second;
        if (bufferId > 0)
        {
            glDeleteBuffers(1, &bufferId);
        }
    }
    _attributeBufferIds.clear();
    for (const auto& indexBufferId : _indexBufferIds)
    {
        unsigned int bufferId = indexBufferId.second;
        if (bufferId > 0)
        {
            glDeleteBuffers(1, &bufferId);
        }
    }
    _indexBufferIds.clear();

    glDeleteVertexArrays(1, &_vertexArray);
    _vertexArray = GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;

    checkGlErrors("after program unbind geometry");
}

ImagePtr GlslProgram::bindTexture(unsigned int uniformType, int uniformLocation, const FilePath& filePath,
                                  ImageHandlerPtr imageHandler, const ImageSamplingProperties& samplingProperties)
{
    if (uniformLocation >= 0 &&
        uniformType >= GL_SAMPLER_1D && uniformType <= GL_SAMPLER_CUBE)
    {
        // Acquire the image.
        string error;
        ImagePtr image = imageHandler->acquireImage(filePath);
        if (imageHandler->bindImage(image, samplingProperties))
        {
            GLTextureHandlerPtr textureHandler = std::static_pointer_cast<GLTextureHandler>(imageHandler);
            int textureLocation = textureHandler->getBoundTextureLocation(image->getResourceId());
            if (textureLocation >= 0)
            {
                glUniform1i(uniformLocation, textureLocation);
            }
        }
        checkGlErrors("after program bind texture");
        return image;
    }

    return nullptr;
}

MaterialX::ValuePtr GlslProgram::findUniformValue(const string& uniformName, const GlslProgram::InputMap& uniformList)
{
    auto uniform = uniformList.find(uniformName);
    if (uniform != uniformList.end())
    {
        int location = uniform->second->location;
        if (location >= 0)
        {
            return uniform->second->value;
        }
    }
    return nullptr;
}

void GlslProgram::bindTextures(ImageHandlerPtr imageHandler)
{
    StringVec errors;
    const string errorType("GLSL texture bind error.");

    if (_programId == UNDEFINED_OPENGL_RESOURCE_ID)
    {
        errors.push_back("Cannot bind textures without a valid program");
        throw ExceptionShaderRenderError(errorType, errors);
    }
    if (!imageHandler)
    {
        errors.push_back("Cannot bind textures without an image handler");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    // Bind textures based on uniforms found in the program
    const GlslProgram::InputMap& uniformList = getUniformsList();
    VariableBlock& publicUniforms = _shader->getStage(Stage::PIXEL).getUniformBlock(HW::PUBLIC_UNIFORMS);
    for (const auto& uniform : uniformList)
    {
        GLenum uniformType = uniform.second->gltype;
        GLint uniformLocation = uniform.second->location;
        if (uniformLocation >= 0 &&
            uniformType >= GL_SAMPLER_1D && uniformType <= GL_SAMPLER_CUBE)
        {
            const string fileName(uniform.second->value ? uniform.second->value->getValueString() : "");

            // Always bind a texture unless it is a lighting texture.
            // Lighting textures are handled in the bindLighting() call.
            // If no texture can be loaded then the default color defined in 
            // "samplingProperties" will be used to create a fallback texture.
            if (fileName != HW::ENV_RADIANCE &&
                fileName != HW::ENV_IRRADIANCE)
            {
                ImageSamplingProperties samplingProperties;
                samplingProperties.setProperties(uniform.first, publicUniforms);
                bindTexture(uniformType, uniformLocation, fileName, imageHandler, samplingProperties);
            }
        }
    }
}

void GlslProgram::bindLighting(LightHandlerPtr lightHandler, ImageHandlerPtr imageHandler)
{
    if (!lightHandler)
    {
        // Nothing to bind if a light handler is not used.
        // This is a valid condition for shaders that don't
        // need lighting so just ignore silently.
        return;
    }

    StringVec errors;

    if (_programId == UNDEFINED_OPENGL_RESOURCE_ID)
    {
        const string errorType("GLSL light binding error.");
        errors.push_back("Cannot bind without a valid program");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    const GlslProgram::InputMap& uniformList = getUniformsList();

    // Bind a couple of lights if can find the light information
    GLint location = GlslProgram::UNDEFINED_OPENGL_PROGRAM_LOCATION;

    // Set the number of active light sources
    size_t lightCount = lightHandler->getLightSources().size();
    auto input = uniformList.find(HW::NUM_ACTIVE_LIGHT_SOURCES);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniform1i(location, int(lightCount));
        }
    }
    else
    {
        // No lighting information so nothing further to do
        lightCount = 0;
    }

    if (lightCount == 0 &&
        !lightHandler->getEnvRadianceMap() &&
        !lightHandler->getEnvIrradianceMap())
    {
        return;
    }

    // Bind environment lights.
    ImageMap envLights =
    {
        { HW::ENV_RADIANCE, lightHandler->getEnvRadianceMap() },
        { HW::ENV_IRRADIANCE, lightHandler->getEnvIrradianceMap() }
    };
    for (const auto& env : envLights)
    {
        auto iblUniform = uniformList.find(env.first);
        GlslProgram::InputPtr inputPtr = iblUniform != uniformList.end() ? iblUniform->second : nullptr;
        if (inputPtr)
        {
            ImagePtr image;
            if (inputPtr->value)
            {
                string filename = inputPtr->value->getValueString();
                if (!filename.empty())
                {
                    image = imageHandler->acquireImage(filename);
                }
            }
            if (!image)
            {
                image = env.second;
            }

            if (image)
            {
                ImageSamplingProperties samplingProperties;
                samplingProperties.uaddressMode = ImageSamplingProperties::AddressMode::PERIODIC;
                samplingProperties.vaddressMode = ImageSamplingProperties::AddressMode::CLAMP;
                samplingProperties.filterType = ImageSamplingProperties::FilterType::LINEAR;

                if (imageHandler->bindImage(image, samplingProperties))
                {
                    GLTextureHandlerPtr textureHandler = std::static_pointer_cast<GLTextureHandler>(imageHandler);
                    int textureLocation = textureHandler->getBoundTextureLocation(image->getResourceId());
                    if (textureLocation >= 0)
                    {
                        glUniform1i(inputPtr->location, textureLocation);
                    }
                    if (iblUniform->first == HW::ENV_RADIANCE)
                    {
                        // This is our radiance texture so set the mip count.
                        auto mipsUniform = uniformList.find(HW::ENV_RADIANCE_MIPS);
                        if (mipsUniform != uniformList.end() && mipsUniform->second->location >= 0)
                        {
                            glUniform1i(mipsUniform->second->location, image->getMaxMipCount());
                        }
                    }
                }
            }
        }
    }

    const vector<NodePtr> lightList = lightHandler->getLightSources();
    const std::unordered_map<string, unsigned int>& ids = lightHandler->getLightIdentifierMap();

    size_t index = 0;
    for (const auto& light : lightList)
    {
        auto nodeDef = light->getNodeDef();
        if (!nodeDef)
        {
            continue;
        }
        const string& nodeDefName = nodeDef->getName();
        const string prefix = HW::LIGHT_DATA_INSTANCE + "[" + std::to_string(index) + "]";

        // Set light type id
        bool boundType = false;
        input = uniformList.find(prefix + ".type");
        if (input != uniformList.end())
        {
            location = input->second->location;
            if (location >= 0)
            {
                auto it = ids.find(nodeDefName);
                if (it != ids.end())
                {
                    glUniform1i(location, it->second);
                    boundType = true;
                }
            }
        }
        if (!boundType)
        {
            continue;
        }

        // Set all inputs
        for (const auto& lightInput : light->getInputs())
        {
            // Make sure we have a value to set
            if (lightInput->hasValue())
            {
                input = uniformList.find(prefix + "." + lightInput->getName());
                if (input != uniformList.end())
                {
                    bindUniformLocation(input->second->location, lightInput->getValue());
                }
            }
        }

        ++index;
    }
}

bool GlslProgram::hasUniform(const string& name)
{
    const GlslProgram::InputMap& uniformList = getUniformsList();
    return uniformList.find(name) != uniformList.end();
}

void GlslProgram::bindUniform(const string& name, ConstValuePtr value, bool errorIfMissing)
{
    const GlslProgram::InputMap& uniformList = getUniformsList();
    auto input = uniformList.find(name);
    if (input != uniformList.end())
    {
        int location = input->second->location;
        if (location < 0)
        {
            if (errorIfMissing)
            {
                throw ExceptionShaderRenderError(
                    "GLSL uniform binding error.",
                    { "Unknown uniform: " + name }
                );
            }
            return;
        }
        bindUniformLocation(location, value);
    }
}

void GlslProgram::bindUniformLocation(int location, ConstValuePtr value)
{
    if (_programId == UNDEFINED_OPENGL_RESOURCE_ID)
    {
        const string errorType("GLSL bind uniform error.");
        StringVec errors;
        errors.push_back("Cannot bind without a valid program");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    if (location >= 0 && value->getValueString() != EMPTY_STRING)
    {
        if (value->getTypeString() == "float")
        {
            float v = value->asA<float>();
            glUniform1f(location, v);
        }
        else if (value->getTypeString() == "integer")
        {
            int v = value->asA<int>();
            glUniform1i(location, v);
        }
        else if (value->getTypeString() == "boolean")
        {
            bool v = value->asA<bool>();
            glUniform1i(location, v ? 1 : 0);
        }
        else if (value->getTypeString() == "color3")
        {
            Color3 v = value->asA<Color3>();
            glUniform3f(location, v[0], v[1], v[2]);
        }
        else if (value->getTypeString() == "color4")
        {
            Color4 v = value->asA<Color4>();
            glUniform4f(location, v[0], v[1], v[2], v[3]);
        }
        else if (value->getTypeString() == "vector2")
        {
            Vector2 v = value->asA<Vector2>();
            glUniform2f(location, v[0], v[1]);
        }
        else if (value->getTypeString() == "vector3")
        {
            Vector3 v = value->asA<Vector3>();
            glUniform3f(location, v[0], v[1], v[2]);
        }
        else if (value->getTypeString() == "vector4")
        {
            Vector4 v = value->asA<Vector4>();
            glUniform4f(location, v[0], v[1], v[2], v[3]);
        }
        else if (value->getTypeString() == "matrix33")
        {
            Matrix33 m = value->asA<Matrix33>();
            glUniformMatrix3fv(location, 1, GL_FALSE, m.data());
        }
        else if (value->getTypeString() == "matrix44")
        {
            Matrix44 m = value->asA<Matrix44>();
            glUniformMatrix4fv(location, 1, GL_FALSE, m.data());
        }
        else
        {
            throw ExceptionShaderRenderError(
                "GLSL input binding error.",
                { "Unsupported data type when setting uniform value" }
            );
        }
    }
}

void GlslProgram::bindViewInformation(ViewHandlerPtr viewHandler)
{
    StringVec errors;
    const string errorType("GLSL view input binding error.");

    if (_programId == UNDEFINED_OPENGL_RESOURCE_ID)
    {
        errors.push_back("Cannot bind without a valid program");
        throw ExceptionShaderRenderError(errorType, errors);
    }
    if (!viewHandler)
    {
        errors.push_back("Cannot bind without a view handler");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    GLint location = GlslProgram::UNDEFINED_OPENGL_PROGRAM_LOCATION;

    // View position and direction
    const GlslProgram::InputMap& uniformList = getUniformsList();
    auto input = uniformList.find(HW::VIEW_POSITION);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniform3fv(location, 1, viewHandler->viewPosition.data());
        }
    }
    input = uniformList.find(HW::VIEW_DIRECTION);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniform3fv(location, 1, viewHandler->viewDirection.data());
        }
    }

    // World matrix variants
    Matrix44& world = viewHandler->worldMatrix;
    Matrix44 invWorld = world.getInverse();
    Matrix44 invTransWorld = invWorld.getTranspose();
    input = uniformList.find(HW::WORLD_MATRIX);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniformMatrix4fv(location, 1, false, world.data());
        }
    }
    input = uniformList.find(HW::WORLD_TRANSPOSE_MATRIX);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniformMatrix4fv(location, 1, false, world.getTranspose().data());
        }
    }
    input = uniformList.find(HW::WORLD_INVERSE_MATRIX);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniformMatrix4fv(location, 1, false, invWorld.data());
        }
    }
    input = uniformList.find(HW::WORLD_INVERSE_TRANSPOSE_MATRIX);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniformMatrix4fv(location, 1, false, invTransWorld.getTranspose().data());
        }
    }

    // Projection matrix variants
    Matrix44& proj = viewHandler->projectionMatrix;
    input = uniformList.find(HW::PROJ_MATRIX);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniformMatrix4fv(location, 1, false, proj.data());
        }
    }
    input = uniformList.find(HW::PROJ_TRANSPOSE_MATRIX);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniformMatrix4fv(location, 1, false, proj.getTranspose().data());
        }
    }
    Matrix44 projInverse= proj.getInverse();
    input = uniformList.find(HW::PROJ_INVERSE_MATRIX);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniformMatrix4fv(location, 1, false, projInverse.data());
        }
    }
    input = uniformList.find(HW::PROJ_INVERSE_TRANSPOSE_MATRIX);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniformMatrix4fv(location, 1, false, projInverse.getTranspose().data());
        }
    }

    // View matrix variants
    Matrix44& view = viewHandler->viewMatrix;
    input = uniformList.find(HW::VIEW_MATRIX);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniformMatrix4fv(location, 1, false, view.data());
        }
    }
    input = uniformList.find(HW::VIEW_TRANSPOSE_MATRIX);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniformMatrix4fv(location, 1, false, view.getTranspose().data());
        }
    }
    Matrix44 viewInverse = view.getInverse();
    input = uniformList.find(HW::VIEW_INVERSE_MATRIX);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniformMatrix4fv(location, 1, false, viewInverse.data());
        }
    }
    input = uniformList.find(HW::VIEW_INVERSE_TRANSPOSE_MATRIX);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniformMatrix4fv(location, 1, false, viewInverse.getTranspose().data());
        }
    }
    
    // View-projection matrix
    Matrix44 viewProj = view * proj;
    input = uniformList.find(HW::VIEW_PROJECTION_MATRIX);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniformMatrix4fv(location, 1, false, viewProj.data());
        }
    }

    // View-projection-world matrix
    Matrix44 viewProjWorld = viewProj * world;
    input = uniformList.find(HW::WORLD_VIEW_PROJECTION_MATRIX);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniformMatrix4fv(location, 1, false, viewProjWorld.data());
        }
    } 
}

void GlslProgram::bindTimeAndFrame()
{
    StringVec errors;
    const string errorType("GLSL input binding error.");

    if (_programId == UNDEFINED_OPENGL_RESOURCE_ID)
    {
        errors.push_back("Cannot bind time/frame without a valid program");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    GLint location = GlslProgram::UNDEFINED_OPENGL_PROGRAM_LOCATION;

    // Bind time
    const GlslProgram::InputMap& uniformList = getUniformsList();
    auto input = uniformList.find(HW::TIME);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniform1f(location, 1.0f);
        }
    }

    // Bind frame
    input = uniformList.find(HW::FRAME);
    if (input != uniformList.end())
    {
        location = input->second->location;
        if (location >= 0)
        {
            glUniform1f(location, 1.0f);
        }
    }
}

bool GlslProgram::hasActiveAttributes() const
{
    GLint activeAttributeCount = 0;
    if (_programId > UNDEFINED_OPENGL_RESOURCE_ID)
    {
        glGetProgramiv(_programId, GL_ACTIVE_ATTRIBUTES, &activeAttributeCount);
    }
    return (activeAttributeCount > 0);
}

void GlslProgram::unbind() const
{
    glUseProgram(0);
}

void GlslProgram::clearInputLists()
{
    _uniformList.clear();
    _attributeList.clear();
}

const GlslProgram::InputMap& GlslProgram::getUniformsList()
{
    return updateUniformsList();
}

const GlslProgram::InputMap& GlslProgram::getAttributesList()
{
    return updateAttributesList();
}

const GlslProgram::InputMap& GlslProgram::updateUniformsList()
{
    StringVec errors;
    const string errorType("GLSL uniform parsing error.");

    if (_uniformList.size() > 0)
    {
        return _uniformList;
    }

    if (_programId <= UNDEFINED_OPENGL_RESOURCE_ID)
    {
        errors.push_back("Cannot parse for uniforms without a valid program");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    // Scan for textures
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
            InputPtr inputPtr = std::make_shared<Input>(uniformLocation, uniformType, uniformSize, EMPTY_STRING);
            _uniformList[string(uniformName)] = inputPtr;
        }
    }
    delete[] uniformName;

    if (_shader)
    {
        // Check for any type mismatches between the program and the h/w shader.
        // i.e the type indicated by the HwShader does not match what was generated.
        bool uniformTypeMismatchFound = false;

        const ShaderStage& ps = _shader->getStage(Stage::PIXEL);
        const ShaderStage& vs = _shader->getStage(Stage::VERTEX);

        // Process constants
        const VariableBlock& constants = ps.getConstantBlock();
        for (size_t i=0; i< constants.size(); ++i)
        {
            const ShaderPort* v = constants[i];
            // There is no way to match with an unnamed variable
            if (v->getVariable().empty())
            {
                continue;
            }

            // TODO: Shoud we really create new ones here each update?
            InputPtr inputPtr = std::make_shared<Input>(-1, -1, int(v->getType()->getSize()), EMPTY_STRING);
            _uniformList[v->getVariable()] = inputPtr;
            inputPtr->isConstant = true;
            inputPtr->value = v->getValue();
            inputPtr->typeString = v->getType()->getName();
            inputPtr->path = v->getPath();
        }

        // Process pixel stage uniforms
        for (const auto& uniformMap : ps.getUniformBlocks())
        {
            const VariableBlock& uniforms = *uniformMap.second;
            if (uniforms.getName() == HW::LIGHT_DATA)
            {
                // Need to go through LightHandler to match with uniforms
                continue;
            }

            for (size_t i = 0; i < uniforms.size(); ++i)
            {
                const ShaderPort* v = uniforms[i];
                int glType = mapTypeToOpenGLType(v->getType());

                // There is no way to match with an unnamed variable
                if (v->getVariable().empty())
                {
                    continue;
                }

                // Ignore types which are unsupported in GLSL.
                if (glType == Input::INVALID_OPENGL_TYPE)
                {
                    continue;
                }

                auto inputIt = _uniformList.find(v->getVariable());
                if (inputIt != _uniformList.end())
                {
                    Input* input = inputIt->second.get();
                    input->path = v->getPath();
                    input->value = v->getValue();
                    if (input->gltype == glType)
                    {
                        input->typeString = v->getType()->getName();
                    }
                    else
                    {
                        errors.push_back(
                            "Pixel shader uniform block type mismatch [" + uniforms.getName() + "]. "
                            + "Name: \"" + v->getVariable()
                            + "\". Type: \"" + v->getType()->getName()
                            + "\". Semantic: \"" + v->getSemantic()
                            + "\". Value: \"" + (v->getValue() ? v->getValue()->getValueString() : "<none>")
                            + "\". GLType: " + std::to_string(mapTypeToOpenGLType(v->getType()))
                        );
                        uniformTypeMismatchFound = true;
                    }
                }
            }
        }

        // Process vertex stage uniforms
        for (const auto& uniformMap : vs.getUniformBlocks())
        {
            const VariableBlock& uniforms = *uniformMap.second;
            for (size_t i = 0; i < uniforms.size(); ++i)
            {
                const ShaderPort* v = uniforms[i];
                auto inputIt = _uniformList.find(v->getVariable());
                if (inputIt != _uniformList.end())
                {
                    Input* input = inputIt->second.get();
                    if (input->gltype == mapTypeToOpenGLType(v->getType()))
                    {
                        input->typeString = v->getType()->getName();
                        input->value = v->getValue();
                        input->path = v->getPath();
                        input->unit = v->getUnit();
                    }
                    else
                    {
                        errors.push_back(
                            "Vertex shader uniform block type mismatch [" + uniforms.getName() + "]. "
                            + "Name: \"" + v->getVariable()
                            + "\". Type: \"" + v->getType()->getName()
                            + "\". Semantic: \"" + v->getSemantic()
                            + "\". Value: \"" + (v->getValue() ? v->getValue()->getValueString() : "<none>")
                            + "\". Unit: \"" + (!v->getUnit().empty() ? v->getUnit() : "<none>")
                            + "\". GLType: " + std::to_string(mapTypeToOpenGLType(v->getType()))
                        );
                        uniformTypeMismatchFound = true;
                    }
                }
            }
        }

        // Throw an error if any type mismatches were found
        if (uniformTypeMismatchFound)
        {
            throw ExceptionShaderRenderError(errorType, errors);
        }
    }

    return _uniformList;
}

int GlslProgram::mapTypeToOpenGLType(const TypeDesc* type)
{
    if (type == Type::INTEGER)
        return GL_INT;
    else if (type == Type::BOOLEAN)
        return GL_BOOL;
    else if (type == Type::FLOAT)
        return GL_FLOAT;
    else if (type->isFloat2())
        return GL_FLOAT_VEC2;
    else if (type->isFloat3())
        return GL_FLOAT_VEC3;
    else if (type->isFloat4())
        return GL_FLOAT_VEC4;
    else if (type == Type::MATRIX33)
        return GL_FLOAT_MAT3;
    else if (type == Type::MATRIX44)
        return GL_FLOAT_MAT4;
    else if (type == Type::FILENAME)
    {
        // A "filename" is not indicative of type, so just return a 2d sampler.
        return GL_SAMPLER_2D;
    }

    return GlslProgram::Input::INVALID_OPENGL_TYPE;
}

const GlslProgram::InputMap& GlslProgram::updateAttributesList()
{
    StringVec errors;
    const string errorType("GLSL attribute parsing error.");

    if (_attributeList.size() > 0)
    {
        return _attributeList;
    }

    if (_programId <= UNDEFINED_OPENGL_RESOURCE_ID)
    {
        errors.push_back("Cannot parse for attributes without a valid program");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    GLint numAttributes = 0;
    GLint maxNameLength = 0;
    glGetProgramiv(_programId, GL_ACTIVE_ATTRIBUTES, &numAttributes);
    glGetProgramiv(_programId, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameLength);
    char* attributeName = new char[maxNameLength];

    for (int i = 0; i < numAttributes; i++)
    {
        GLint attributeSize = 0;
        GLenum attributeType = 0;
        glGetActiveAttrib(_programId, GLuint(i), maxNameLength, nullptr, &attributeSize, &attributeType, attributeName);
        GLint attributeLocation = glGetAttribLocation(_programId, attributeName);
        if (attributeLocation >= 0)
        {
            InputPtr inputPtr = std::make_shared<Input>(attributeLocation, attributeType, attributeSize, EMPTY_STRING);

            // Attempt to pull out the set number for specific attributes
            //
            string sattributeName(attributeName);
            const string colorSet(HW::IN_COLOR + "_");
            const string uvSet(HW::IN_TEXCOORD + "_");
            if (string::npos != sattributeName.find(colorSet))
            {
                string setNumber = sattributeName.substr(colorSet.size(), sattributeName.size());
                inputPtr->value = Value::createValueFromStrings(setNumber, getTypeString<int>());
            }
            else if (string::npos != sattributeName.find(uvSet))
            {
                string setNumber = sattributeName.substr(uvSet.size(), sattributeName.size());
                inputPtr->value = Value::createValueFromStrings(setNumber, getTypeString<int>());
            }

            _attributeList[sattributeName] = inputPtr;
        }
    }
    delete[] attributeName;

    if (_shader)
    {
        const ShaderStage& vs = _shader->getStage(Stage::VERTEX);

        bool uniformTypeMismatchFound = false;

        const VariableBlock& vertexInputs = vs.getInputBlock(HW::VERTEX_INPUTS);
        if (!vertexInputs.empty())
        {
            for (size_t i = 0; i < vertexInputs.size(); ++i)
            {
                const ShaderPort* v = vertexInputs[i];
                auto inputIt = _attributeList.find(v->getVariable());
                if (inputIt != _attributeList.end())
                {
                    Input* input = inputIt->second.get();
                    input->value = v->getValue();
                    if (input->gltype == mapTypeToOpenGLType(v->getType()))
                    {
                        input->typeString = v->getType()->getName();
                    }
                    else
                    {
                        errors.push_back(
                            "Vertex shader attribute type mismatch in block. Name: \"" + v->getVariable()
                            + "\". Type: \"" + v->getType()->getName()
                            + "\". Semantic: \"" + v->getSemantic()
                            + "\". Value: \"" + (v->getValue() ? v->getValue()->getValueString() : "<none>")
                            + "\". GLType: " + std::to_string(mapTypeToOpenGLType(v->getType()))
                        );
                        uniformTypeMismatchFound = true;
                    }
                }
            }
        }

        // Throw an error if any type mismatches were found
        if (uniformTypeMismatchFound)
        {
            throw ExceptionShaderRenderError(errorType, errors);
        }
    }

    return _attributeList;
}

void GlslProgram::findInputs(const string& variable,
                             const InputMap& variableList,
                             InputMap& foundList,
                             bool exactMatch)
{
    foundList.clear();

    // Scan all attributes which match the attribute identifier completely or as a prefix
    //
    int ilocation = UNDEFINED_OPENGL_PROGRAM_LOCATION;
    auto input = variableList.find(variable);
    if (input != variableList.end())
    {
        ilocation = input->second->location;
        if (ilocation >= 0)
        {
            foundList[variable] = input->second;
        }
    }
    else if (!exactMatch)
    {
        for (input = variableList.begin(); input != variableList.end(); ++input)
        {
            const string& name = input->first;
            if (name.compare(0, variable.size(), variable) == 0)
            {
                ilocation = input->second->location;
                if (ilocation >= 0)
                {
                    foundList[input->first] = input->second;
                }
            }
        }
    }
}

void GlslProgram::printUniforms(std::ostream& outputStream)
{
    updateUniformsList();
    for (const auto& input : _uniformList)
    {
        unsigned int gltype = input.second->gltype;
        int location = input.second->location;
        int size = input.second->size;
        string type = input.second->typeString;
        string value = input.second->value ? input.second->value->getValueString() : EMPTY_STRING;
        string unit = input.second->unit;
        bool isConstant = input.second->isConstant;
        outputStream << "Program Uniform: \"" << input.first
            << "\". Location:" << location
            << ". GLtype: " << std::hex << gltype
            << ". Size: " << std::dec << size;
        if (!type.empty())
            outputStream << ". TypeString: \"" << type << "\"";
        if (!value.empty())
        {
            outputStream << ". Value: " << value;
            if (!unit.empty())
                outputStream << ". Unit: " << unit;
        }
        outputStream << ". Is constant: " << isConstant;
        if (!input.second->path.empty())
            outputStream << ". Element Path: \"" << input.second->path << "\"";
        outputStream << "." << std::endl;
    }
}


void GlslProgram::printAttributes(std::ostream& outputStream)
{
    updateAttributesList();
    for (const auto& input : _attributeList)
    {
        unsigned int gltype = input.second->gltype;
        int location = input.second->location;
        int size = input.second->size;
        string type = input.second->typeString;
        string value = input.second->value ? input.second->value->getValueString() : EMPTY_STRING;
        outputStream << "Program Attribute: \"" << input.first
            << "\". Location:" << location
            << ". GLtype: " << std::hex << gltype
            << ". Size: " << std::dec << size;
        if (!type.empty())
            outputStream << ". TypeString: \"" << type << "\"";
        if (!value.empty())
            outputStream << ". Value: " << value;
        outputStream << "." << std::endl;
    }
}

} // namespace MaterialX
