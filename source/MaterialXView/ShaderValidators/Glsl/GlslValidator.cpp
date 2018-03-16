
#include <MaterialXView/External/GLew/glew.h>
#include <MaterialXView/ShaderValidators/Glsl/GlslValidator.h>
#include <MaterialXView/Handlers/DefaultGeometryHandler.h>

#include <iostream>
#include <algorithm>

namespace MaterialX
{
// View information
const float NEAR_PLANE = -100.0f;
const float FAR_PLANE = 100.0f;

//
// Creator
//
GlslValidatorPtr GlslValidator::creator()
{
    return std::shared_ptr<GlslValidator>(new GlslValidator());
}

GlslValidator::GlslValidator() :
    ShaderValidator(),
    _colorTarget(0),
    _depthTarget(0),
    _frameBuffer(0),
    _frameBufferWidth(512),
    _frameBufferHeight(512),
    _indexBuffer(0),
    _indexBufferSize(0),
    _vertexArray(0),
    _dummyTexture(0),
    _initialized(false),
    _window(nullptr),
    _context(nullptr)
{
    // Clear buffer ids to invalid identifier.
    _attributeBufferIds.resize(ATTRIBUTE_COUNT);
    std::fill(_attributeBufferIds.begin(), _attributeBufferIds.end(), MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);

    _program = GlslProgram::creator();
    _geometryHandler = DefaultGeometryHandler::creator();
}

GlslValidator::~GlslValidator()
{
    // Clean up the program 
    _program = nullptr;
    
    // Clean up offscreen target
    deleteTarget();

    // Clean up the context
    _context = nullptr;

    // Clean up the window
    _window = nullptr;
}

void GlslValidator::initialize()
{
    ShaderValidationErrorList errors;
    const std::string errorType("OpenGL utilities initialization.");

    if (!_initialized)
    {
        // Create window
        _window = SimpleWindow::creator();

        const char* windowName = "Validator Window";
        bool created = _window->initialize(const_cast<char *>(windowName),
                                          _frameBufferWidth, _frameBufferHeight,
                                          nullptr);
        if (!created)
        {
            errors.push_back("Failed to create window for testing.");
            throw ExceptionShaderValidationError(errorType, errors);
        }
        else
        {
            // Create offscreen context
            _context = GLUtilityContext::creator(_window->windowWrapper(), nullptr);
            if (!_context)
            {
                errors.push_back("Failed to create OpenGL context for testing.");
                throw ExceptionShaderValidationError(errorType, errors);
            }
            else
            {
                if (_context->makeCurrent())
                {
                    // Initialize glew
                    bool initializedFunctions = true;

                    glewInit();
                    if (!glewIsSupported("GL_VERSION_4_0"))
                    {
                        initializedFunctions = false;
                        errors.push_back("OpenGL version 4.0 not supported");
                        throw ExceptionShaderValidationError(errorType, errors);
                    }

                    if (initializedFunctions)
                    {
                        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
                        glClearStencil(0);

                        _initialized = true;
                    }
                }
            }
        }
    }
}

void GlslValidator::deleteTarget()
{
    if (_frameBuffer)
    {
        if (_context && _context->makeCurrent())
        {
            glBindFramebuffer(GL_FRAMEBUFFER, MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
            glDeleteTextures(1, &_colorTarget);
            glDeleteTextures(1, &_depthTarget);
            glDeleteFramebuffers(1, &_frameBuffer);
        }
    }
}

bool GlslValidator::createTarget()
{
    ShaderValidationErrorList errors;
    const std::string errorType("OpenGL target creation failure.");

    if (!_context)
    {
        errors.push_back("No valid OpenGL context to create target with.");
        throw ExceptionShaderValidationError(errorType, errors);
    }
    if (!_context->makeCurrent())
    {
        errors.push_back("Cannot make OpenGL context current to create target with.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Only frame buffer only once
    if (_frameBuffer > 0)
    {
        return true;
    }

    // Set up frame buffer
    glGenFramebuffers(1, &_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);

    // Create an offscreen floating point color target and attach to the framebuffer
    _colorTarget = MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
    glGenTextures(1, &_colorTarget);
    glBindTexture(GL_TEXTURE_2D, _colorTarget);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _frameBufferWidth, _frameBufferHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTarget, 0);

    // Create floating point offscreen depth target
    _depthTarget = MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
    glGenTextures(1, &_depthTarget);
    glBindTexture(GL_TEXTURE_2D, _depthTarget);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, _frameBufferWidth, _frameBufferHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTarget, 0);
    
    glBindTexture(GL_TEXTURE_2D, MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
    glDrawBuffer(GL_NONE);

    // Validate the framebuffer. Default to fixed point if we cannot get
    // a floating point buffer.
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        glDeleteTextures(1, &_colorTarget);
        glGenTextures(1, &_colorTarget);
        glBindTexture(GL_TEXTURE_2D, _colorTarget);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _frameBufferWidth, _frameBufferHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
        glBindTexture(GL_TEXTURE_2D, MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTarget, 0);
        // Re-check status again.
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    }
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
        glDeleteFramebuffers(1, &_frameBuffer);
        _frameBuffer = MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;

        std::string errorMessage("Frame buffer object setup failed: ");
        switch (status) {
        case GL_FRAMEBUFFER_COMPLETE:
            errorMessage += "GL_FRAMEBUFFER_COMPLETE";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            errorMessage += "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            errorMessage += "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            errorMessage += "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            errorMessage += "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            errorMessage += "GL_FRAMEBUFFER_UNSUPPORTED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            errorMessage += "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
            break;
        case GL_FRAMEBUFFER_UNDEFINED:
            errorMessage += "GL_FRAMEBUFFER_UNDEFINED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            errorMessage += "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
            break;
        default:
            errorMessage += std::to_string(status);
            break;
        }

        errors.push_back(errorMessage);
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Unbind on cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);

    return true;
}

bool GlslValidator::bindTarget(bool bind)
{
    // Make sure we have a target to bind first
    createTarget();

    // Bind the frame buffer and route to color texture target
    if (bind)
    {
        if (!_frameBuffer)
        {
            ShaderValidationErrorList errors;
            errors.push_back("No framebuffer exists to bind.");
            throw ExceptionShaderValidationError("OpenGL target bind failure.", errors);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
        GLenum colorList[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, colorList);
    }
    // Unbind frame buffer and route nowhere.
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
        glDrawBuffer(GL_NONE);
    }
    return true;
}

void GlslValidator::validateCreation(const ShaderPtr shader)
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL program creation error.");

    if (!_context)
    {
        errors.push_back("No valid OpenGL context to create program with.");
        throw ExceptionShaderValidationError(errorType, errors);

    }
    if (!_context->makeCurrent())
    {
        errors.push_back("Cannot make OpenGL context current to create program.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    _program->setStages(std::dynamic_pointer_cast<HwShader>(shader));
    _program->build();    
}

void GlslValidator::validateCreation(const std::vector<std::string>& stages)
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL program creation error.");

    if (!_context)
    {
        errors.push_back("No valid OpenGL context to create program with.");
        throw ExceptionShaderValidationError(errorType, errors);

    }
    if (!_context->makeCurrent())
    {
        errors.push_back("Cannot make OpenGL context current to create program.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    _program->setStages(stages);
    _program->build();
}

void GlslValidator::validateInputs()
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL program input error.");

    if (!_context)
    {
        errors.push_back("No valid OpenGL context to validate inputs.");
        throw ExceptionShaderValidationError(errorType, errors);

    }
    if (!_context->makeCurrent())
    {
        errors.push_back("Cannot make OpenGL context current to validate inputs.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Check that the generated uniforms and attributes are valid
    _program->getUniformsList();
    _program->getAttributesList();
}

////////////////////////////////////////////////////////////////////////////////////
// Binders
////////////////////////////////////////////////////////////////////////////////////
void GlslValidator::bindTimeAndFrame()
{
    ShaderValidationErrorList errors;

    if (!_program)
    {
        const std::string errorType("GLSL input binding error.");
        errors.push_back("Cannot bind time/frame without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    GLint location = MaterialX::GlslProgram::UNDEFINED_OPENGL_PROGRAM_LOCATION;

    // Bind time
    const MaterialX::GlslProgram::InputMap& uniformList = _program->getUniformsList();
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

void GlslValidator::bindLighting()
{
    if (!_lightHandler)
    {
        // Nothing to bind if a light handler is not used.
        // This is a valid condition for shaders that don't 
        // need lighting so just ignore silently.
        return;
    }

    ShaderValidationErrorList errors;

    if (!_program)
    {
        const std::string errorType("GLSL light binding error.");
        errors.push_back("Cannot bind without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Bind a couple of lights if can find the light information
    GLint location = MaterialX::GlslProgram::UNDEFINED_OPENGL_PROGRAM_LOCATION;

    // Set the number of active light sources
    size_t lightCount = _lightHandler->getLightSources().size();
    const MaterialX::GlslProgram::InputMap& uniformList = _program->getUniformsList();
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
    for (LightSourcePtr light : _lightHandler->getLightSources())
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
                    setUniform(input->second->location, *param.second);
                }
            }
        }

        ++index;
    }
}

void GlslValidator::setUniform(int location, const Value& value)
{
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

void GlslValidator::bindViewInformation()
{
    ShaderValidationErrorList errors;

    if (!_program)
    {
        const std::string errorType("GLSL input binding error.");
        errors.push_back("Cannot bind without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    GLint location = MaterialX::GlslProgram::UNDEFINED_OPENGL_PROGRAM_LOCATION;

    // Set view direction and position
    const MaterialX::GlslProgram::InputMap& uniformList = _program->getUniformsList();
    auto Input = uniformList.find("u_viewPosition");
    if (Input != uniformList.end())
    {
        location = Input->second->location;
        if (location >= 0)
        {
            glUniform3f(location, 0.0f, 0.0f, NEAR_PLANE-1.0f);
        }
    }
    Input = uniformList.find("u_viewDirection");
    if (Input != uniformList.end())
    {
        location = Input->second->location;
        if (location >= 0)
        {
            glUniform3f(location, 0.0f, 0.0f, 1.0f);
        }
    }

    GLfloat mvm[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mvm);

    GLfloat pm[16];
    glGetFloatv(GL_PROJECTION_MATRIX, pm);

    // Set world related matrices
    //
    std::vector<std::string> worldMatrixVariables =
    {
        "u_worldMatrix",
        //"u_worldInverseMatrix",
        "u_worldTransposeMatrix",
        //"u_worldInverseTransposeMatrix"
    };
    for (auto worldMatrixVariable : worldMatrixVariables)
    {
        Input = uniformList.find(worldMatrixVariable);
        if (Input != uniformList.end())
        {
            location = Input->second->location;
            if (location >= 0)
            {
                bool transpose = (worldMatrixVariable.find("Transpose") != std::string::npos);
                glUniformMatrix4fv(location, 1, transpose, mvm);
            }
        }
    }

    // Bind projection matrices
    //
    std::vector<std::string> projectionMatrixVariables =
    {
        "u_projectionMatrix",
        //"u_projectionInverseMatrix",
        "u_projectionTransposeMatrix",
        //"u_projectionInverseTransposeMatrix",
    };
    for (auto projectionMatrixVariable : projectionMatrixVariables)
    {
        Input = uniformList.find(projectionMatrixVariable);
        if (Input != uniformList.end())
        {
            location = Input->second->location;
            if (location >= 0)
            {
                bool transpose = (projectionMatrixVariable.find("Transpose") != std::string::npos);
                glUniformMatrix4fv(location, 1, transpose, pm);
            }
        }
    }

    // Bind view related matrices
    std::vector<std::string> viewMatrixVariables =
    {
        //"u_viewMatrix",
        //"u_viewInverseMatrix",
        //"u_viewTransposeMatrix",
        //"u_viewInverseTransposeMatrix",
        "u_viewProjectionMatrix"
        //"u_worldViewProjectionMatrix"
    };
    for (auto viewMatrixVariable : viewMatrixVariables)
    {
        Input = uniformList.find(viewMatrixVariable);
        if (Input != uniformList.end())
        {
            location = Input->second->location;
            if (location >= 0)
            {
                glUniformMatrix4fv(location, 1, GL_FALSE, pm);
            }
        }
    }
    
    checkErrors();
}

void GlslValidator::bindAttribute(const float* bufferData,
                                    size_t bufferSize,                                    
                                    const GlslValidator::AttributeIndex attributeIndex,
                                    unsigned int floatCount,
                                    const MaterialX::GlslProgram::InputMap& inputs)
{
    for (auto input : inputs)
    {
        int location = input.second->location;
        if (_attributeBufferIds[attributeIndex] < 1)
        {
            // Create a buffer based on attribute type.
            unsigned int buffer = MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
            glGenBuffers(1, &buffer);
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glBufferData(GL_ARRAY_BUFFER, bufferSize, bufferData, GL_STATIC_DRAW);
            _attributeBufferIds[attributeIndex] = buffer;
        }

        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, floatCount, GL_FLOAT, GL_FALSE, 0, 0);
    }
}

void GlslValidator::bindGeometry()
{
    ShaderValidationErrorList errors;
    if (!_program)
    {
        const std::string errorType("GLSL matrix bind error.");
        errors.push_back("Cannot bind geometry without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    MaterialX::GlslProgram::InputMap foundList;
    const MaterialX::GlslProgram::InputMap& attributeList = _program->getAttributesList();

    // Set up vertex arrays
    glGenVertexArrays(1, &_vertexArray);
    glBindVertexArray(_vertexArray);

    size_t bufferSize = 0;
    GeometryHandler::IndexBuffer& indexData = _geometryHandler->getIndexing(bufferSize);
    _indexBufferSize = indexData.size();
    glGenBuffers(1, &_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizei)bufferSize, &indexData[0], GL_STATIC_DRAW);

    GeometryHandler::InputProperties properties(_frameBufferWidth, _frameBufferHeight, 20);
    _geometryHandler->setInputProperties(properties);

    // Bind positions
    _program->findInputs("i_position", attributeList, foundList, true);
    if (foundList.size())
    {
        GeometryHandler::FloatBuffer& positionData = _geometryHandler->getPositions(bufferSize);
        bindAttribute(&positionData[0], bufferSize, POSITION3_ATTRIBUTE, 3, foundList);
    }

    // Bind normals
    _program->findInputs("i_normal", attributeList, foundList, true);
    if (foundList.size())
    {
        GeometryHandler::FloatBuffer& normalData = _geometryHandler->getNormals(bufferSize);
        bindAttribute(&normalData[0], bufferSize, NORMAL3_ATTRIBUTE, 3, foundList);
    }

    // Bind tangents
    _program->findInputs("i_tangent", attributeList, foundList, true);
    if (foundList.size())
    {
        GeometryHandler::FloatBuffer& tangentData = _geometryHandler->getTangents(bufferSize, 0);
        bindAttribute(&tangentData[0], bufferSize, TANGENT3_ATTRIBUTE, 3, foundList);
    }

    // Bind bitangents
    _program->findInputs("i_bitangent", attributeList, foundList, true);
    if (foundList.size())
    {
        GeometryHandler::FloatBuffer& bitangentData = _geometryHandler->getBitangents(bufferSize, 0);
        bindAttribute(&bitangentData[0], bufferSize, BITANGENT3_ATTRIBUTE, 3, foundList);
    }

    // Bind single set of colors for all locations found
    _program->findInputs("i_color_", attributeList, foundList, false);
    if (foundList.size())
    {
        GeometryHandler::FloatBuffer& colorData = _geometryHandler->getColors(bufferSize, 0);
        bindAttribute(&colorData[0], bufferSize, COLOR4_ATTRIBUTE, 4, foundList);
    }

    // Bind single set of texture coords for all locations found
    // Search for anything that starts with the prefix "i_texcoord_"
    _program->findInputs("i_texcoord_", attributeList, foundList, false);
    if (foundList.size())
    {
        GeometryHandler::FloatBuffer& uvData = _geometryHandler->getTextureCoords(bufferSize, 0);
        bindAttribute(&uvData[0], bufferSize, TEXCOORD2_ATTRIBUTE, 2, foundList);
    }

    // Bind any named attribute information
    //
    std::string geomAttrPrefix("u_geomattr_");
    const MaterialX::GlslProgram::InputMap& uniformList = _program->getUniformsList();
    _program->findInputs(geomAttrPrefix, uniformList, foundList, false);
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

void GlslValidator::createDummyTexture()
{
    if (_dummyTexture == MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID)
    {
        unsigned int width = 0;
        unsigned int height = 0;
        unsigned char* pixels = nullptr;
        if (_imageHandler)
        {
            _imageHandler->createDefaultImage(width, height, &pixels);
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

void GlslValidator::unbindTextures()
{
    int textureUnit = 0;
    GLint maxImageUnits = -1;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxImageUnits);
    const MaterialX::GlslProgram::InputMap& uniformList = _program->getUniformsList();
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

void GlslValidator::bindTextures()
{
    if (!_program)
    {
        const std::string errorType("GLSL matrix bind error.");
        ShaderValidationErrorList errors;
        errors.push_back("Cannot bind textures without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Bind textures based on uniforms found in the program
    int textureUnit = 0;
    GLint maxImageUnits = -1;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxImageUnits);
    // Keep track of all textures read in
    _programTextures.clear();
    const MaterialX::GlslProgram::InputMap& uniformList = _program->getUniformsList();
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
            if (!fileName.empty() && _imageHandler)
            {
                unsigned int width = 0;
                unsigned int height = 0;
                unsigned int channelCount = 0;
                float* buffer = nullptr;
                if (_imageHandler->loadImage(fileName, width, height, channelCount, &buffer) &&
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
                createDummyTexture();
                glBindTexture(GL_TEXTURE_2D, _dummyTexture); // Bind a dummy texture
            }

            textureUnit++;
        }
    }

    checkErrors();
}


void GlslValidator::unbindGeometry()
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
    for (unsigned int i=0; i<ATTRIBUTE_COUNT; i++)
    {
        unsigned int bufferId = _attributeBufferIds[i];
        if (bufferId > 0)
        {
            glDeleteBuffers(1, &bufferId);
            _attributeBufferIds[i] = MaterialX::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
        }
    }

    checkErrors();
}

void GlslValidator::bindInputs()
{
    // Bind the program to use
    if (!_program->bind())
    {
        const std::string errorType("GLSL bind inputs error.");
        ShaderValidationErrorList errors;
        errors.push_back("Cannot bind inputs without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    checkErrors();

    // Parse for uniforms and attributes
    _program->getUniformsList();
    _program->getAttributesList();

    // Bind based on inputs found
    bindViewInformation();
    bindGeometry();
    bindTextures();
    bindTimeAndFrame();
    bindLighting();
}

void GlslValidator::validateRender()
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL rendering error.");

    if (!_context)
    {
        errors.push_back("No valid OpenGL context to render to.");
        throw ExceptionShaderValidationError(errorType, errors);
    }
    if (!_context->makeCurrent())
    {
        errors.push_back("Cannot make OpenGL context current to render to.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Set up target
    bindTarget(true);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up viewing / projection matrices for an orthographic rendering
    glViewport(0, 0, _frameBufferWidth, _frameBufferHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, _frameBufferWidth, 0.0f, _frameBufferHeight, NEAR_PLANE, FAR_PLANE);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    try
    {
        // Bind program and input parameters
        if (_program)
        {
            // Check if we have any attributes to bind. If not then
            // there is nothing to draw
            if (!_program->haveActiveAttributes())
            {
                errors.push_back("Program has no input vertex data.");
                throw ExceptionShaderValidationError(errorType, errors);
            }
            else
            {
                // Bind the program to use
                _program->bind();
                checkErrors();

                bindInputs();

                glDrawElements(GL_TRIANGLES, (GLsizei)_indexBufferSize, GL_UNSIGNED_INT, (void*)0);
                checkErrors();

                // Unbind resources
                _program->unbind();
                unbindTextures();
                unbindGeometry();
            }
        }

        // Fallack draw some simple geometry
        else
        {
            glPushMatrix();
            glBegin(GL_QUADS);

            glTexCoord2f(0.0f, 1.0f);
            glNormal3f(1.0f, 0.0f, 0.0f);
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex2f(0.0f, (float)_frameBufferHeight);

            glTexCoord2f(0.0f, 0.0f);
            glNormal3f(1.0f, 0.0, 0.0);
            glColor3f(0.0f, 1.0f, 0.0f);
            glVertex2f(0.0f, 0.0f);

            glTexCoord2f(1.0f, 0.0f);
            glNormal3f(1.0f, 0.0, 0.0);
            glColor3f(0.0f, 0.0f, 1.0f);
            glVertex2f((float)_frameBufferWidth, 0.0f);

            glTexCoord2f(1.0f, 1.0f);
            glNormal3f(1.0f, 0.0, 0.0);
            glColor3f(1.0f, 1.0f, 0.0f);
            glVertex2f((float)_frameBufferWidth, (float)_frameBufferHeight);

            glEnd();
            glPopMatrix();

            checkErrors();
        }
    }
    catch (ExceptionShaderValidationError e)
    {
        bindTarget(false);
        throw e;
    }

    // Unset target
    bindTarget(false);
}

void GlslValidator::save(const std::string& fileName)
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL image save error.");

    if (!_imageHandler)
    {
        errors.push_back("No image handler specified.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    size_t bufferSize = _frameBufferWidth * _frameBufferHeight * 4;
    float* buffer = new float[bufferSize];
    if (!buffer)
    {
        errors.push_back("Failed to read color buffer back.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Read back from the color texture.
    bindTarget(true);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, _frameBufferWidth, _frameBufferHeight, GL_RGBA, GL_FLOAT, buffer);
    bindTarget(false);
    try
    {
        checkErrors();
    }
    catch (ExceptionShaderValidationError e)
    {
        delete[] buffer;
        errors.push_back("Failed to read color buffer back.");
        errors.insert(std::end(errors), std::begin(e.errorLog()), std::end(e.errorLog()));
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Save using the handler
    bool saved = _imageHandler->saveImage(fileName, _frameBufferWidth, _frameBufferHeight, 4, buffer);
    delete[] buffer;

    if (!saved)
    {
        errors.push_back("Faled to save to file:" + fileName);
        throw ExceptionShaderValidationError(errorType, errors);
    }
}

void GlslValidator::checkErrors()
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
