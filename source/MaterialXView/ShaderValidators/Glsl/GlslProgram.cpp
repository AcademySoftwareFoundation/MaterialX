
#include <MaterialXView/External/GLew/glew.h>
#include <MaterialXView/ShaderValidators/Glsl/GlslProgram.h>

#include <iostream>
#include <algorithm>
#include <cmath>

namespace MaterialX
{
// OpenGL Constants
unsigned int GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID = 0;
int GlslProgram::UNDEFINED_OPENGL_PROGRAM_LOCATION = -1;
int GlslProgram::Input::INVALID_OPENGL_TYPE = -1;

//
// Creator
//
GlslProgramPtr GlslProgram::creator()
{
    return std::shared_ptr<GlslProgram>(new GlslProgram());
}

GlslProgram::GlslProgram() :
    _programId(UNDEFINED_OPENGL_RESOURCE_ID),
    _hwShader(nullptr),
    _indexBuffer(0),
    _indexBufferSize(0),
    _vertexArray(0),
    _dummyTexture(0)
{
}

GlslProgram::~GlslProgram()
{
    // Clean up the program and offscreen target
    deleteProgram();
}

void GlslProgram::setStages(const std::vector<std::string>& stages)
{
    if (stages.size() != HwShader::NUM_STAGES)
    {
        ShaderValidationErrorList errors;
        throw ExceptionShaderValidationError("Incorrect number of stages passed in for stage setup.", errors);
    }

    unsigned int count = 0;
    for (auto stage : stages)
    { 
        _stages[count++] = stage;
    }

    // A stage change invalidates any cached parsed inputs
    clearInputLists();
    _hwShader = nullptr;
}


void GlslProgram::setStages(const HwShaderPtr shader)
{
    if (!shader)
    {
        ShaderValidationErrorList errors;
        throw ExceptionShaderValidationError("Cannot set stages using null hardware shader.", errors);
    }

    // Clear out any old data
    clearStages();

    // Extract out the shader code per stage
    _hwShader = shader;
    for (size_t i = 0; i < HwShader::NUM_STAGES; i++)
    {
        _stages[i] = _hwShader->getSourceCode(i);
    }

    // A stage change invalidates any cached parsed inputs
    clearInputLists();
}

const std::string GlslProgram::getStage(size_t stage) const
{
    if (stage < HwShader::NUM_STAGES)
    {
        return _stages[stage];
    }
    return std::string();
}

void GlslProgram::clearStages()
{
    for (size_t i = 0; i < HwShader::NUM_STAGES; i++)
    {
        _stages[i].clear();
    }

    // Clearing stages invalidates any cached inputs
    clearInputLists();
}

bool GlslProgram::haveValidStages() const
{
    // Need at least a pixel shader stage and a vertex shader stage
    const std::string& vertexShaderSource = _stages[HwShader::VERTEX_STAGE];
    const std::string& fragmentShaderSource = _stages[HwShader::PIXEL_STAGE];

    return (vertexShaderSource.length() > 0 && fragmentShaderSource.length() > 0);
}

void GlslProgram::deleteProgram()
{
    if (_programId > UNDEFINED_OPENGL_RESOURCE_ID)
    {
        glUseProgram(0);
        glDeleteObjectARB(_programId);
        _programId = UNDEFINED_OPENGL_RESOURCE_ID;
    }

    // Program deleted, so also clear cached input lists
    clearInputLists();
}

unsigned int GlslProgram::build()
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL program creation error.");

    deleteProgram();

    if (!haveValidStages())
    {
        errors.push_back("An invalid set of stages has been provided.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    GLint GLStatus = GL_FALSE;
    int GLInfoLogLength = 0;

    unsigned int stagesBuilt = 0;
    unsigned int desiredStages = 0;
    for (unsigned int i = 0; i < HwShader::NUM_STAGES; i++)
    {
        if (_stages[i].length())
            desiredStages++;
    }

    // Create vertex shader
    GLuint vertexShaderId = UNDEFINED_OPENGL_RESOURCE_ID;
    std::string &vertexShaderSource = _stages[HwShader::VERTEX_STAGE];
    if (vertexShaderSource.length())
    {
        vertexShaderId = glCreateShader(GL_VERTEX_SHADER);

        // Compile vertex shader
        const char* vertexChar = vertexShaderSource.c_str();
        glShaderSource(vertexShaderId, 1, &vertexChar, NULL);
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
                glGetShaderInfoLog(vertexShaderId, GLInfoLogLength, NULL,
                    &vsErrorMessage[0]);
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
    std::string& fragmentShaderSource = _stages[HwShader::PIXEL_STAGE];
    if (fragmentShaderSource.length())
    {
        fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

        // Compile fragment shader
        const char *fragmentChar = fragmentShaderSource.c_str();
        glShaderSource(fragmentShaderId, 1, &fragmentChar, NULL);
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
                glGetShaderInfoLog(fragmentShaderId, GLInfoLogLength, NULL,
                    &fsErrorMessage[0]);
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
                glGetProgramInfoLog(_programId, GLInfoLogLength, NULL,
                    &ProgramErrorMessage[0]);
                errors.push_back(&ProgramErrorMessage[0]);
            }
        }
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
    if (errors.size())
    {
        throw ExceptionShaderValidationError(errorType, errors);
    }

    return _programId;
}

bool GlslProgram::bind()
{
    if (_programId > UNDEFINED_OPENGL_RESOURCE_ID)
    {
        glUseProgram(_programId);
        checkErrors();
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
        const std::string errorType("GLSL bind inputs error.");
        ShaderValidationErrorList errors;
        errors.push_back("Cannot bind inputs without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    checkErrors();

    // Parse for uniforms and attributes
    getUniformsList();
    getAttributesList();

    // Bind based on inputs found
    bindViewInformation(viewHandler);
    bindGeometry(geometryHandler);
    bindTextures(imageHandler);
    bindTimeAndFrame();
    bindLighting(lightHandler);
}

void GlslProgram::unbindInputs()
{
    unbindTextures();
    unbindGeometry();
}

void GlslProgram::bindAttribute(const MaterialX::GlslProgram::InputMap& inputs, GeometryHandlerPtr geometryHandler)
{
    const std::string errorType("GLSL bind attribute error.");
    ShaderValidationErrorList errors;

    if (!geometryHandler)
    {
        errors.push_back("Cannot bind geometry without a geometry handler");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    const size_t FLOAT_SIZE = sizeof(float);

    for (auto input : inputs)
    {
        int location = input.second->location;
        unsigned int index = input.second->value ? input.second->value->asA<int>() : 0;

        unsigned int stride = 0;
        GeometryHandler::FloatBuffer& attributeData = geometryHandler->getAttribute(input.first, stride, index);

        if (attributeData.empty() || (stride == 0))
        {
            errors.push_back("Geometry buffer could not be retrieved for binding: " + input.first);
            throw ExceptionShaderValidationError(errorType, errors);
        }

        if (_attributeBufferIds.find(input.first) == _attributeBufferIds.end())
        {
            const float* bufferData = &attributeData[0];
            size_t bufferSize = attributeData.size()*FLOAT_SIZE;

            // Create a buffer based on attribute type.
            unsigned int bufferId = MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
            glGenBuffers(1, &bufferId);
            glBindBuffer(GL_ARRAY_BUFFER, bufferId);
            glBufferData(GL_ARRAY_BUFFER, bufferSize, bufferData, GL_STATIC_DRAW);

            _attributeBufferIds[input.first] = bufferId;
        }

        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, stride, GL_FLOAT, GL_FALSE, 0, 0);
    }
}

void GlslProgram::bindGeometry(GeometryHandlerPtr geometryHandler)
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL geometry bind error.");

    if (_programId == UNDEFINED_OPENGL_RESOURCE_ID)
    {
        errors.push_back("Cannot bind geometry without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }
    if (!geometryHandler)
    {
        errors.push_back("Cannot bind geometry without a geometry handler");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    MaterialX::GlslProgram::InputMap foundList;
    const MaterialX::GlslProgram::InputMap& attributeList = getAttributesList();

    // Set up vertex arrays
    glGenVertexArrays(1, &_vertexArray);
    glBindVertexArray(_vertexArray);

    size_t UINT_SIZE = sizeof(unsigned int);
    GeometryHandler::IndexBuffer& indexData = geometryHandler->getIndexing();
    _indexBufferSize = indexData.size();
    glGenBuffers(1, &_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (unsigned int)(_indexBufferSize*UINT_SIZE), &indexData[0], GL_STATIC_DRAW);

    // Bind positions
    findInputs("i_position", attributeList, foundList, true);
    if (foundList.size())
    {
        bindAttribute(foundList, geometryHandler);
    }

    // Bind normals
    findInputs("i_normal", attributeList, foundList, true);
    if (foundList.size())
    {
        bindAttribute(foundList, geometryHandler);
    }

    // Bind tangents
    findInputs("i_tangent", attributeList, foundList, true);
    if (foundList.size())
    {
        bindAttribute(foundList, geometryHandler);
    }

    // Bind bitangents
    findInputs("i_bitangent", attributeList, foundList, true);
    if (foundList.size())
    {
        bindAttribute(foundList, geometryHandler);
    }

    // Bind colors
    // Search for anything that starts with the prefix "i_color_"
    findInputs("i_color_", attributeList, foundList, false);
    if (foundList.size())
    {
        bindAttribute(foundList, geometryHandler);
    }

    // Bind texture coordinates
    // Search for anything that starts with the prefix "i_texcoord_"
    findInputs("i_texcoord_", attributeList, foundList, false);
    if (foundList.size())
    {
        bindAttribute(foundList, geometryHandler);
    }

    // Bind any named attribute information
    //
    std::string geomAttrPrefix("u_geomattr_");
    const MaterialX::GlslProgram::InputMap& uniformList = getUniformsList();
    findInputs(geomAttrPrefix, uniformList, foundList, false);
    for (auto Input : foundList)
    {
        // Only handle float1-4 types for now
        if (Input.second->gltype == GL_FLOAT)
        {
            GLfloat floatVal[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            int size = Input.second->size;
            if (size == 1)
                glUniform1fv(Input.second->location, 1, floatVal);
            else if (size == 2)
                glUniform2fv(Input.second->location, 1, floatVal);
            else if (size == 3)
                glUniform3fv(Input.second->location, 1, floatVal);
            else if (size == 4)
                glUniform4fv(Input.second->location, 1, floatVal);
        }
    }

    checkErrors();
}

void GlslProgram::unbindGeometry()
{
    // Cleanup attribute bindings
    //
    glBindVertexArray(0);
    int numberAttributes = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numberAttributes);
    for (int i = 0; i < numberAttributes; i++)
    {
        glDisableVertexAttribArray(i);
    }
    glBindBuffer(GL_ARRAY_BUFFER, MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);

    // Clean up buffers
    //
    if (_indexBuffer > 0)
    {
        glDeleteBuffers(1, &_indexBuffer);
        _indexBuffer = MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
    }
    for (auto attributeBufferId : _attributeBufferIds)
    {
        unsigned int bufferId = attributeBufferId.second;
        if (bufferId > 0)
        {
            glDeleteBuffers(1, &bufferId);
        }
    }
    _attributeBufferIds.clear();

    checkErrors();
}

void GlslProgram::unbindTextures()
{
    int textureUnit = 0;
    GLint maxImageUnits = -1;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxImageUnits);
    const MaterialX::GlslProgram::InputMap& uniformList = getUniformsList();
    for (auto uniform : uniformList)
    {
        GLenum uniformType = uniform.second->gltype;
        GLint uniformLocation = uniform.second->location;
        if (uniformLocation >= 0 &&
            uniformType >= GL_SAMPLER_1D && uniformType <= GL_SAMPLER_CUBE)
        {
            if (textureUnit >= maxImageUnits)
            {
                break;
            }

            // Unbind a texture to that unit
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
            checkErrors();
            textureUnit++;
        }
    }

    // Delete any allocated textures
    if (_dummyTexture != MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID)
    {
        glDeleteTextures(1, &_dummyTexture);
        _dummyTexture = MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
    }
    for (auto id : _programTextures)
    {
        if (id != MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID)
        {
            glDeleteTextures(1, &id);
        }
    }
    _programTextures.clear();

    checkErrors();
}

void GlslProgram::createDummyTexture(ImageHandlerPtr imageHandler)
{
    if (_dummyTexture == MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID)
    {
        unsigned int width = 0;
        unsigned int height = 0;
        unsigned char* pixels = nullptr;
        if (imageHandler)
        {
            imageHandler->createDefaultImage(width, height, &pixels);
        }

        if ((width * height > 0) && pixels)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glGenTextures(1, &_dummyTexture);

            glBindTexture(GL_TEXTURE_2D, _dummyTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            // Note: Must do this for default sampling to lookup properly.
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
        }
    }
}

void GlslProgram::bindTextures(ImageHandlerPtr imageHandler)
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL texture bind error.");

    if (_programId == UNDEFINED_OPENGL_RESOURCE_ID)
    {
        errors.push_back("Cannot bind textures without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }
    if (!imageHandler)
    {
        errors.push_back("Cannot bind textures without an image handler");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Bind textures based on uniforms found in the program
    int textureUnit = 0;
    GLint maxImageUnits = -1;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxImageUnits);
    // Keep track of all textures read in
    _programTextures.clear();
    const MaterialX::GlslProgram::InputMap& uniformList = getUniformsList();
    for (auto uniform : uniformList)
    {
        GLenum uniformType = uniform.second->gltype;
        GLint uniformLocation = uniform.second->location;
        if (uniformLocation >= 0 &&
            uniformType >= GL_SAMPLER_1D && uniformType <= GL_SAMPLER_CUBE)
        {
            if (textureUnit >= maxImageUnits)
            {
                break;
            }

            // Map location to a texture unit incrementally
            glUniform1i(uniformLocation, textureUnit);
            // Bind a texture to that unit
            glActiveTexture(GL_TEXTURE0 + textureUnit);

            bool textureBound = false;
            std::string fileName(uniform.second->value ? uniform.second->value->getValueString() : "");
            if (!fileName.empty() && imageHandler)
            {
                unsigned int width = 0;
                unsigned int height = 0;
                unsigned int channelCount = 0;
                float* buffer = nullptr;
                if (imageHandler->loadImage(fileName, width, height, channelCount, &buffer) &&
                    (channelCount == 3 || channelCount == 4))
                {
                    unsigned int newTexture = MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                    glGenTextures(1, &newTexture);
                    // Keep track of texture created
                    _programTextures.push_back(newTexture);

                    glBindTexture(GL_TEXTURE_2D, newTexture);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                        0, (channelCount == 4 ? GL_RGBA : GL_RGB), GL_FLOAT, buffer);
                    glGenerateMipmap(GL_TEXTURE_2D);

                    textureBound = true;
                }
            }

            if (!textureBound)
            {
                createDummyTexture(imageHandler);
                glBindTexture(GL_TEXTURE_2D, _dummyTexture); // Bind a dummy texture
            }

            textureUnit++;
        }
    }

    checkErrors();
}


void GlslProgram::bindLighting(LightHandlerPtr lightHandler)
{
    if (!lightHandler)
    {
        // Nothing to bind if a light handler is not used.
        // This is a valid condition for shaders that don't 
        // need lighting so just ignore silently.
        return;
    }

    ShaderValidationErrorList errors;

    if (_programId == UNDEFINED_OPENGL_RESOURCE_ID)
    {
        const std::string errorType("GLSL light binding error.");
        errors.push_back("Cannot bind without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Bind a couple of lights if can find the light information
    GLint location = MaterialX::GlslProgram::UNDEFINED_OPENGL_PROGRAM_LOCATION;

    // Set the number of active light sources
    size_t lightCount = lightHandler->getLightSources().size();
    const MaterialX::GlslProgram::InputMap& uniformList = getUniformsList();
    auto input = uniformList.find("u_numActiveLightSources");
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

    if (lightCount == 0)
    {
        return;
    }

    size_t index = 0;
    for (LightSourcePtr light : lightHandler->getLightSources())
    {
        const string prefix = "u_lightData[" + std::to_string(index) + "]";

        // Set light type id
        input = uniformList.find(prefix + ".type");
        if (input != uniformList.end())
        {
            location = input->second->location;
            if (location >= 0)
            {
                glUniform1i(location, int(light->getType()));
            }
        }

        // Set all parameters
        for (auto param : light->getParameters())
        {
            // Make sure we have a value to set
            if (param.second)
            {
                input = uniformList.find(prefix + "." + param.first);
                if (input != uniformList.end())
                {
                    bindUniform(input->second->location, *param.second);
                }
            }
        }

        ++index;
    }
}

void GlslProgram::bindUniform(int location, const Value& value)
{
    if (_programId == UNDEFINED_OPENGL_RESOURCE_ID)
    {
        const std::string errorType("GLSL bind uniform error.");
        ShaderValidationErrorList errors;
        errors.push_back("Cannot bind without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    if (location >= 0 && value.getValueString() != EMPTY_STRING)
    {
        if (value.getTypeString() == "float")
        {
            float v = value.asA<float>();
            glUniform1f(location, v);
        }
        else if (value.getTypeString() == "integer")
        {
            int v = value.asA<int>();
            glUniform1i(location, v);
        }
        else if (value.getTypeString() == "boolean")
        {
            bool v = value.asA<bool>();
            glUniform1i(location, v ? 1 : 0);
        }
        else if (value.getTypeString() == "color2")
        {
            Color2 v = value.asA<Color2>();
            glUniform2f(location, v[0], v[1]);
        }
        else if (value.getTypeString() == "color3")
        {
            Color3 v = value.asA<Color3>();
            glUniform3f(location, v[0], v[1], v[2]);
        }
        else if (value.getTypeString() == "color4")
        {
            Color4 v = value.asA<Color4>();
            glUniform4f(location, v[0], v[1], v[2], v[3]);
        }
        else if (value.getTypeString() == "vector2")
        {
            Vector2 v = value.asA<Vector2>();
            glUniform2f(location, v[0], v[1]);
        }
        else if (value.getTypeString() == "vector3")
        {
            Vector3 v = value.asA<Vector3>();
            glUniform3f(location, v[0], v[1], v[2]);
        }
        else if (value.getTypeString() == "vector4")
        {
            Vector4 v = value.asA<Vector4>();
            glUniform4f(location, v[0], v[1], v[2], v[3]);
        }
        else
        {
            throw ExceptionShaderValidationError(
                "GLSL input binding error.",
                { "Unsupported data type when setting uniform value" }
            );
        }
    }
}


void GlslProgram::bindViewInformation(ViewHandlerPtr viewHandler)
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL view input binding error.");

    if (_programId == UNDEFINED_OPENGL_RESOURCE_ID)
    {
        errors.push_back("Cannot bind without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }
    if (!viewHandler)
    {
        errors.push_back("Cannot bind without a view handler");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    GLint location = MaterialX::GlslProgram::UNDEFINED_OPENGL_PROGRAM_LOCATION;

    // Set view direction and position
    const MaterialX::GlslProgram::InputMap& uniformList = getUniformsList();
    auto Input = uniformList.find("u_viewPosition");
    if (Input != uniformList.end())
    {
        location = Input->second->location;
        if (location >= 0)
        {
            Vector3& viewPosition = viewHandler->viewPosition();
            glUniform3f(location, viewPosition[0], viewPosition[1], viewPosition[2]);
        }
    }
    Input = uniformList.find("u_viewDirection");
    if (Input != uniformList.end())
    {
        location = Input->second->location;
        if (location >= 0)
        {
            Vector3& viewDirection = viewHandler->viewDirection();
            glUniform3f(location, viewDirection[0], viewDirection[1], viewDirection[2]);
        }
    }

    Matrix44& worldMatrix = viewHandler->worldMatrix();
    Matrix44& viewMatrix = viewHandler->viewMatrix();
    Matrix44& projectionMatrix = viewHandler->projectionMatrix();
    Matrix44 viewProjection = viewMatrix * projectionMatrix;

    // Set world related matrices. World matrix is identity so
    // bind the same matrix to all locations
    //
    std::vector<std::string> worldMatrixVariables =
    {
        "u_worldMatrix",
        "u_worldInverseMatrix",
        "u_worldTransposeMatrix",
        "u_worldInverseTransposeMatrix"
    };
    for (auto worldMatrixVariable : worldMatrixVariables)
    {
        Input = uniformList.find(worldMatrixVariable);
        if (Input != uniformList.end())
        {
            location = Input->second->location;
            if (location >= 0)
            {
                glUniformMatrix4fv(location, 1, false, &(worldMatrix[0][0]));
            }
        }
    }

    // Bind projection matrices
    //
    std::vector<std::string> projectionMatrixVariables =
    {
        "u_projectionMatrix",
        "u_projectionTransposeMatrix",
        "u_projectionInverseMatrix",
        "u_projectionInverseTransposeMatrix",
    };
    Matrix44 inverseProjection;
    bool computedInverse = false;
    for (auto projectionMatrixVariable : projectionMatrixVariables)
    {
        Input = uniformList.find(projectionMatrixVariable);
        if (Input != uniformList.end())
        {
            location = Input->second->location;
            if (location >= 0)
            {
                bool transpose = (projectionMatrixVariable.find("Transpose") != std::string::npos);
                if (projectionMatrixVariable.find("Inverse") != std::string::npos)
                {
                    if (!computedInverse)
                    {
                        viewHandler->invertMatrix(projectionMatrix, inverseProjection);
                    }
                    glUniformMatrix4fv(location, 1, transpose, &(inverseProjection[0][0]));
                }
                else
                {
                    glUniformMatrix4fv(location, 1, transpose, &(projectionMatrix[0][0]));
                }
            }
        }
    }

    // Bind view related matrices
    std::vector<std::string> viewMatrixVariables =
    {
        "u_viewMatrix",
        "u_viewTransposeMatrix",
        "u_viewInverseMatrix",
        "u_viewInverseTransposeMatrix",
    };
    Matrix44 inverseView;
    computedInverse = false;
    for (auto viewMatrixVariable : viewMatrixVariables)
    {
        Input = uniformList.find(viewMatrixVariable);
        if (Input != uniformList.end())
        {
            location = Input->second->location;
            if (location >= 0)
            {
                bool transpose = (viewMatrixVariable.find("Transpose") != std::string::npos);
                if (viewMatrixVariable.find("Inverse") != std::string::npos)
                {
                    if (!computedInverse)
                    {
                        viewHandler->invertMatrix(viewMatrix, inverseView);
                    }
                    glUniformMatrix4fv(location, 1, transpose, &(inverseView[0][0]));
                }
                else
                {
                    glUniformMatrix4fv(location, 1, transpose, &(viewMatrix[0][0]));
                }
            }
        }
    }

    // Bind combined matrices
    std::vector<std::string> combinedMatrixVariables =
    {
        "u_viewProjectionMatrix",
        "u_worldViewProjectionMatrix"
    };
    for (auto combinedMatrixVariable : combinedMatrixVariables)
    {
        Input = uniformList.find(combinedMatrixVariable);
        if (Input != uniformList.end())
        {
            location = Input->second->location;
            if (location >= 0)
            {
                glUniformMatrix4fv(location, 1, GL_FALSE, &(viewProjection[0][0]));
            }
        }
    }

    checkErrors();
}

void GlslProgram::bindTimeAndFrame()
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL input binding error.");

    if (_programId == UNDEFINED_OPENGL_RESOURCE_ID)
    {
        errors.push_back("Cannot bind time/frame without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    GLint location = MaterialX::GlslProgram::UNDEFINED_OPENGL_PROGRAM_LOCATION;

    // Bind time
    const MaterialX::GlslProgram::InputMap& uniformList = getUniformsList();
    auto Input = uniformList.find("u_time");
    if (Input != uniformList.end())
    {
        location = Input->second->location;
        if (location >= 0)
        {
            glUniform1f(location, 1.0f);
        }
    }

    // Bind frame
    Input = uniformList.find("u_frame");
    if (Input != uniformList.end())
    {
        location = Input->second->location;
        if (location >= 0)
        {
            glUniform1f(location, 1.0f);
        }
    }
}


bool GlslProgram::haveActiveAttributes() const
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
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL uniform parsing error.");

    if (_uniformList.size() > 0)
    {
        return _uniformList;
    }

    if (_programId <= UNDEFINED_OPENGL_RESOURCE_ID)
    {
        errors.push_back("Cannot parse for uniforms without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
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
            InputPtr inputPtr = std::make_shared<Input>(uniformLocation, uniformType, uniformSize);
            _uniformList[std::string(uniformName)] = inputPtr;
        }
    }
    delete[] uniformName;

    if (_hwShader)
    {
        // Check for any type mismatches between the program and the h/w shader.
        // i.e the type indicated by the HwShader does not match what was generated.
        bool uniformTypeMismatchFound = false;

        /// Return all blocks of uniform variables for a stage.
        const std::string LIGHT_DATA_STRING("LightData");
        const MaterialX::Shader::VariableBlockMap& pixelShaderUniforms = _hwShader->getUniformBlocks(HwShader::PIXEL_STAGE);
        for (auto uniforms : pixelShaderUniforms)
        {
            MaterialX::Shader::VariableBlockPtr block = uniforms.second;
            if (block->name == LIGHT_DATA_STRING)
            {
                // Need to go through LightHandler to match with uniforms
                continue;
            }

            for (const MaterialX::Shader::Variable* input : block->variableOrder)
            {
                // There is no way to match with an unnamed variable
                if (input->name.empty())
                {
                    continue;
                }

                auto Input = _uniformList.find(input->name);
                if (Input != _uniformList.end())
                {
                    if (input->value)
                    {
                        Input->second->value = input->value;
                    }
                    if (Input->second->gltype == mapTypeToOpenGLType(input->type))
                    {
                        Input->second->typeString = input->type;
                    }
                    else
                    {
                        errors.push_back(
                            "Pixel shader uniform block type mismatch[" + uniforms.first + "]. "
                            + "Name: \"" + input->name
                            + "\". Type: \"" + input->type
                            + "\". Semantic: \"" + input->semantic
                            + "\". Value: \"" + (input->value ? input->value->getValueString() : "<none>")
                            + "\". GLType: " + std::to_string(mapTypeToOpenGLType(input->type))
                        );
                        uniformTypeMismatchFound = true;
                    }
                }
            }
        }

        const MaterialX::Shader::VariableBlockMap& vertexShaderUniforms = _hwShader->getUniformBlocks(HwShader::VERTEX_STAGE);
        for (auto uniforms : vertexShaderUniforms)
        {
            MaterialX::Shader::VariableBlockPtr block = uniforms.second;
            for (const MaterialX::Shader::Variable* input : block->variableOrder)
            {
                auto Input = _uniformList.find(input->name);
                if (Input != _uniformList.end())
                {
                    if (Input->second->gltype == mapTypeToOpenGLType(input->type))
                    {
                        Input->second->typeString = input->type;
                        Input->second->value = input->value;
                        Input->second->typeString = input->type;
                    }
                    else
                    {
                        errors.push_back(
                            "Vertex shader uniform block type mismatch[" + uniforms.first + "]. "
                            + "Name: \"" + input->name
                            + "\". Type: \"" + input->type
                            + "\". Semantic: \"" + input->semantic
                            + "\". Value: \"" + (input->value ? input->value->getValueString() : "<none>")
                            + "\". GLType: " + std::to_string(mapTypeToOpenGLType(input->type))
                        );
                        uniformTypeMismatchFound = true;
                    }
                }
            }
        }

        // Throw an error if any type mismatches were found
        if (uniformTypeMismatchFound)
        {
            ExceptionShaderValidationError(errorType, errors);
        }
    }

    return _uniformList;
}

int GlslProgram::mapTypeToOpenGLType(const std::string& type)
{
    if (type == MaterialX::getTypeString<int>())
        return GL_INT;
    else if (type == MaterialX::getTypeString<bool>())
        return GL_BOOL;
    else if (type == MaterialX::getTypeString<float>())
        return GL_FLOAT;
    else if (type == MaterialX::getTypeString<Vector2>() || type == MaterialX::getTypeString<Color2>())
        return GL_FLOAT_VEC2;
    else if (type == MaterialX::getTypeString<Vector3>() || type == MaterialX::getTypeString<Color3>())
        return GL_FLOAT_VEC3;
    else if (type == MaterialX::getTypeString<Vector4>() || type == MaterialX::getTypeString<Color4>())
        return GL_FLOAT_VEC4;
    else if (type == MaterialX::getTypeString<Matrix33>())
        return GL_FLOAT_MAT3;
    else if (type == MaterialX::getTypeString<Matrix44>())
        return GL_FLOAT_MAT4;
    else if (type == MaterialX::FILENAME_TYPE_STRING)
    {
        // A "filename" is not indicative of type, so just return a 2d sampler.
        return GL_SAMPLER_2D;
    }

    return GlslProgram::Input::INVALID_OPENGL_TYPE;
}

const GlslProgram::InputMap& GlslProgram::updateAttributesList()
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL attribute parsing error.");

    if (_attributeList.size() > 0)
    {
        return _attributeList;
    }

    if (_programId <= UNDEFINED_OPENGL_RESOURCE_ID)
    {
        errors.push_back("Cannot parse for attributes without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
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
            InputPtr inputPtr = std::make_shared<Input>(attributeLocation, attributeType, attributeSize);

            // Attempt to pull out the set number for specific attributes 
            //
            std::string sattributeName(attributeName);
            const std::string colorSet("i_color_");
            const std::string uvSet("i_texcoord_");
            if (std::string::npos != sattributeName.find(colorSet))
            {
                std::string setNumber = sattributeName.substr(colorSet.size(), sattributeName.size());
                inputPtr->value = MaterialX::Value::createValueFromStrings(setNumber, MaterialX::getTypeString<int>());
            }
            else if (std::string::npos != sattributeName.find(uvSet))
            {
                std::string setNumber = sattributeName.substr(uvSet.size(), sattributeName.size());
                inputPtr->value = MaterialX::Value::createValueFromStrings(setNumber, MaterialX::getTypeString<int>());
            }

            _attributeList[sattributeName] = inputPtr;
        }
    }
    delete[] attributeName;

    if (_hwShader)
    {        
        const MaterialX::Shader::VariableBlock& appDataBlock = _hwShader->getAppDataBlock();
        
        bool uniformTypeMismatchFound = false;

        if (appDataBlock.variableOrder.size())
        { 
            for (const MaterialX::Shader::Variable* input : appDataBlock.variableOrder)
            {
                auto Input = _attributeList.find(input->name);
                if (Input != _attributeList.end())
                {
                    if (input->value)
                    {
                        Input->second->value = input->value;
                    }

                    if (Input->second->gltype == mapTypeToOpenGLType(input->type))
                    {
                        Input->second->typeString = input->type;
                    }
                    else
                    {
                        errors.push_back(
                            "Application uniform type mismatch in block. Name: \"" + input->name
                            + "\". Type: \"" + input->type
                            + "\". Semantic: \"" + input->semantic
                            + "\". Value: \"" + (input->value ? input->value->getValueString() : "<none>")
                            + "\". GLType: " + std::to_string(mapTypeToOpenGLType(input->type))
                        );
                        uniformTypeMismatchFound = true;
                    }
                }
            }
        }

        // Throw an error if any type mismatches were found
        if (uniformTypeMismatchFound)
        {
            ExceptionShaderValidationError(errorType, errors);
        }
    }

    return _attributeList;
}

void GlslProgram::findInputs(const std::string& variable, 
                                         const InputMap& variableList,
                                         InputMap& foundList,
                                         bool exactMatch)
{
    foundList.clear();

    // Scan all attributes which match the attribute identifier completely or as a prefix
    //
    int ilocation = UNDEFINED_OPENGL_PROGRAM_LOCATION;
    auto Input = variableList.find(variable);
    if (Input != variableList.end())
    {
        ilocation = Input->second->location;
        if (ilocation >= 0)
        {
            foundList[variable] = Input->second;
        }
    }
    else if (!exactMatch)
    {
        for (Input = variableList.begin(); Input != variableList.end(); Input++)
        {
            const std::string& name = Input->first;
            if (name.compare(0, variable.size(), variable) == 0)
            {
                ilocation = Input->second->location;
                if (ilocation >= 0)
                {
                    foundList[Input->first] = Input->second;
                }
            }
        }
    }
}

void GlslProgram::printUniforms(std::ostream& outputStream) 
{
    updateUniformsList();
    for (auto input : _uniformList)
    {
        unsigned int gltype = input.second->gltype;
        int location = input.second->location;
        int size = input.second->size;
        std::string type = input.second->typeString;
        std::string value = input.second->value ? input.second->value->getValueString() : "<none>";
        outputStream << "Program Uniform: \"" << input.first
            << "\". Location:" << location
            << ". GLtype: " << std::hex << gltype
            << ". Size: " << std::dec << size
            << ". TypeString:" << type
            << ". Value: " << value << "."
            << std::endl;
    }
}


void GlslProgram::printAttributes(std::ostream& outputStream) 
{
    updateAttributesList();
    for (auto input : _attributeList)
    {
        unsigned int gltype = input.second->gltype;
        int location = input.second->location;
        int size = input.second->size;
        std::string type = input.second->typeString;
        std::string value = input.second->value ? input.second->value->getValueString() : "<none>";
        outputStream << "Program Attribute: \"" << input.first
            << "\". Location=" << location
            << ". Type=" << std::hex << gltype
            << ". Size=" << std::dec << size
            << ". TypeString=" << type
            << ". Value=" << value << "."
            << std::endl;
    }
}

void GlslProgram::checkErrors()
{
    ShaderValidationErrorList errors;

    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        errors.push_back("OpenGL error: " + std::to_string(error));
    }
    if (errors.size())
    {
        throw ExceptionShaderValidationError("OpenGL context error.", errors);
    }
}

}
