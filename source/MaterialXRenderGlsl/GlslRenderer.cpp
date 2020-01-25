//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRenderGlsl/External/GLew/glew.h>
#include <MaterialXRenderGlsl/GlslRenderer.h>
#include <MaterialXRenderGlsl/GLUtilityContext.h>
#include <MaterialXRenderHw/SimpleWindow.h>
#include <MaterialXRender/TinyObjLoader.h>

#include <iostream>

namespace MaterialX
{

const float PI = std::acos(-1.0f);

// View information
const float FOV_PERSP = 45.0f; // degrees
const float NEAR_PLANE_PERSP = 0.05f;
const float FAR_PLANE_PERSP = 100.0f;

//
// GlslRenderer methods
//

GlslRendererPtr GlslRenderer::create(unsigned int res)
{
    return GlslRendererPtr(new GlslRenderer(res));
}

GlslRenderer::GlslRenderer(unsigned int res) :
    _colorTarget(0),
    _depthTarget(0),
    _frameBuffer(0),
    _frameBufferWidth(res),
    _frameBufferHeight(res),
    _initialized(false),
    _window(nullptr),
    _context(nullptr)
{
    _program = GlslProgram::create();

    TinyObjLoaderPtr loader = TinyObjLoader::create();
    _geometryHandler = GeometryHandler::create();
    _geometryHandler->addLoader(loader);

    _viewHandler = ViewHandler::create();
}

GlslRenderer::~GlslRenderer()
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

void GlslRenderer::initialize()
{
    StringVec errors;
    const string errorType("OpenGL utilities initialization.");

    if (!_initialized)
    {
        // Create window
        _window = SimpleWindow::create();

        const char* windowName = "Renderer Window";
        bool created = _window->initialize(const_cast<char *>(windowName),
                                          _frameBufferWidth, _frameBufferHeight,
                                          nullptr);
        if (!created)
        {
            errors.push_back("Failed to create window for testing.");
            throw ExceptionShaderRenderError(errorType, errors);
        }
        else
        {
            // Create offscreen context
            _context = GLUtilityContext::create(_window->windowWrapper(), nullptr);
            if (!_context)
            {
                errors.push_back("Failed to create OpenGL context for testing.");
                throw ExceptionShaderRenderError(errorType, errors);
            }
            else
            {
                if (_context->makeCurrent())
                {
                    // Initialize glew
                    glewInit();
#if !defined(__APPLE__)
                    if (!glewIsSupported("GL_VERSION_4_0"))
                    {
                        errors.push_back("OpenGL version 4.0 not supported");
                        throw ExceptionShaderRenderError(errorType, errors);
                    }
#endif
                    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
                    glClearStencil(0);

                    _initialized = true;
                }
            }
        }
    }
}

void GlslRenderer::deleteTarget()
{
    if (_frameBuffer)
    {
        if (_context && _context->makeCurrent())
        {
            glBindFramebuffer(GL_FRAMEBUFFER, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
            glDeleteTextures(1, &_colorTarget);
            glDeleteTextures(1, &_depthTarget);
            glDeleteFramebuffers(1, &_frameBuffer);
        }
    }
}

bool GlslRenderer::createTarget()
{
    StringVec errors;
    const string errorType("OpenGL target creation failure.");

    if (!_context)
    {
        errors.push_back("No valid OpenGL context to create target with.");
        throw ExceptionShaderRenderError(errorType, errors);
    }
    if (!_context->makeCurrent())
    {
        errors.push_back("Cannot make OpenGL context current to create target with.");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    // Only frame buffer only once
    if (_frameBuffer > 0)
    {
        return true;
    }

    // Set up frame buffer
    glGenFramebuffers(1, &_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);

    // Create an offscreen sRGB color target and attach to the framebuffer
    _colorTarget = GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
    glGenTextures(1, &_colorTarget);
    glBindTexture(GL_TEXTURE_2D, _colorTarget);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, _frameBufferWidth, _frameBufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTarget, 0);

    // Create floating point offscreen depth target
    _depthTarget = GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;
    glGenTextures(1, &_depthTarget);
    glBindTexture(GL_TEXTURE_2D, _depthTarget);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, _frameBufferWidth, _frameBufferHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTarget, 0);

    glBindTexture(GL_TEXTURE_2D, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
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
        glBindTexture(GL_TEXTURE_2D, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTarget, 0);
        // Re-check status again.
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    }
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
        glDeleteFramebuffers(1, &_frameBuffer);
        _frameBuffer = GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;

        string errorMessage("Frame buffer object setup failed: ");
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
        throw ExceptionShaderRenderError(errorType, errors);
    }

    // Unbind on cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);

    return true;
}

bool GlslRenderer::bindTarget(bool bind)
{
    // Make sure we have a target to bind first
    createTarget();

    // Bind the frame buffer and route to color texture target
    if (bind)
    {
        if (!_frameBuffer)
        {
            StringVec errors;
            errors.push_back("No framebuffer exists to bind.");
            throw ExceptionShaderRenderError("OpenGL target bind failure.", errors);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
        GLenum colorList[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, colorList);
    }
    // Unbind frame buffer and route nowhere.
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
        glDrawBuffer(GL_NONE);
    }
    return true;
}

void GlslRenderer::createProgram(ShaderPtr shader)
{
    StringVec errors;
    const string errorType("GLSL program creation error.");

    if (!_context)
    {
        errors.push_back("No valid OpenGL context to create program with.");
        throw ExceptionShaderRenderError(errorType, errors);

    }
    if (!_context->makeCurrent())
    {
        errors.push_back("Cannot make OpenGL context current to create program.");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    _program->setStages(shader);
    _program->build();
}

void GlslRenderer::createProgram(const StageMap& stages)
{
    StringVec errors;
    const string errorType("GLSL program creation error.");

    if (!_context)
    {
        errors.push_back("No valid OpenGL context to create program with.");
        throw ExceptionShaderRenderError(errorType, errors);

    }
    if (!_context->makeCurrent())
    {
        errors.push_back("Cannot make OpenGL context current to create program.");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    for (const auto& it : stages)
    {
        _program->addStage(it.first, it.second);
    }
    _program->build();
}

void GlslRenderer::renderTextureSpace(bool encodeSrgb)
{
    bindTarget(true);
    if (encodeSrgb)
    {
        glEnable(GL_FRAMEBUFFER_SRGB);
    }
    else
    {
        glDisable(GL_FRAMEBUFFER_SRGB);
    }
    glViewport(0, 0, _frameBufferWidth, _frameBufferHeight);

    _program->bind();

    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    _program->bindTextures(_imageHandler);

    glBindVertexArray(vao);
    float vertices[] =
    {
         // positions       // texcoords
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f
    };
    unsigned int indices[] =
    {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    bindTarget(false);
    checkErrors();

    _program->unbind();
    _program->unbindInputs(_imageHandler);
}

void GlslRenderer::validateInputs()
{
    StringVec errors;
    const string errorType("GLSL program input error.");

    if (!_context)
    {
        errors.push_back("No valid OpenGL context to validate inputs.");
        throw ExceptionShaderRenderError(errorType, errors);
    }
    if (!_context->makeCurrent())
    {
        errors.push_back("Cannot make OpenGL context current to validate inputs.");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    // Check that the generated uniforms and attributes are valid
    _program->getUniformsList();
    _program->getAttributesList();
}

void GlslRenderer::updateViewInformation(const Vector3& eye,
                                           const Vector3& center,
                                           const Vector3& up,                                          
                                           float viewAngle,
                                           float nearDist,
                                           float farDist,
                                           float objectScale)
{
    float fH = std::tan(viewAngle / 360.0f * PI) * nearDist;
    float fW = fH * 1.0f;

    Vector3 boxMin = _geometryHandler->getMinimumBounds();
    Vector3 boxMax = _geometryHandler->getMaximumBounds();
    Vector3 sphereCenter = (boxMax + boxMin) / 2.0;
    float sphereRadius = (sphereCenter - boxMin).getMagnitude();
    float meshFit = 2.0f / sphereRadius;
    Vector3 modelTranslation = sphereCenter * -1.0f;

    Matrix44& world = _viewHandler->worldMatrix;
    Matrix44& view = _viewHandler->viewMatrix;
    Matrix44& proj = _viewHandler->projectionMatrix;
    view = ViewHandler::createViewMatrix(eye, center, up);
    proj = ViewHandler::createPerspectiveMatrix(-fW, fW, -fH, fH, nearDist, farDist);
    world = Matrix44::createScale(Vector3(objectScale * meshFit));
    world *= Matrix44::createTranslation(modelTranslation).getTranspose();

    Matrix44 invView = view.getInverse();
    _viewHandler->viewDirection = { invView[0][2], invView[1][2], invView[2][2] };
    _viewHandler->viewPosition = { invView[0][3], invView[1][3], invView[2][3] };
}

void GlslRenderer::render()
{
    StringVec errors;
    const string errorType("GLSL rendering error.");

    if (!_context)
    {
        errors.push_back("No valid OpenGL context to render to.");
        throw ExceptionShaderRenderError(errorType, errors);
    }
    if (!_context->makeCurrent())
    {
        errors.push_back("Cannot make OpenGL context current to render to.");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    // Set up target
    bindTarget(true);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, _frameBufferWidth, _frameBufferHeight);

    // Update viewing information
    const Vector3 eye(0.0f, 0.0f, 4.0f);
    const Vector3 center;
    const Vector3 up(0.0f, 1.0f, 0.0f);
    float objectScale(1.0f);
    updateViewInformation(eye, center, up, FOV_PERSP, NEAR_PLANE_PERSP, FAR_PLANE_PERSP, objectScale);

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
                throw ExceptionShaderRenderError(errorType, errors);
            }
            else
            {
                // Bind the program to use
                _program->bind();
                _program->bindInputs(_viewHandler, _geometryHandler, _imageHandler, _lightHandler);

                // Draw all the partitions of all the meshes in the handler
                for (const auto& mesh : _geometryHandler->getMeshes())
                {
                    for (size_t i = 0; i < mesh->getPartitionCount(); i++)
                    {
                        auto part = mesh->getPartition(i);
                        _program->bindPartition(part);

                        MeshIndexBuffer& indexData = part->getIndices();
                        glDrawElements(GL_TRIANGLES, (GLsizei)indexData.size(), GL_UNSIGNED_INT, (void*)0);
                    }
                }
                checkErrors();

                // Unbind resources
                _program->unbind();
                _program->unbindInputs(_imageHandler);
            }
        }
    }
    catch (ExceptionShaderRenderError& /*e*/)
    {
        bindTarget(false);
        throw;
    }

    // Unset target
    bindTarget(false);
}

void GlslRenderer::save(const FilePath& filePath, bool floatingPoint)
{
    StringVec errors;
    const string errorType("GLSL image save error.");

    if (!_imageHandler)
    {
        errors.push_back("No image handler specified.");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    // Create an image.
    ImagePtr image = Image::create(_frameBufferWidth, _frameBufferHeight, 4);
    image->createResourceBuffer();

    // Read back from the color texture.
    bindTarget(true);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, image->getWidth(), image->getHeight(), GL_RGBA,
                 floatingPoint ? GL_FLOAT : GL_UNSIGNED_BYTE, image->getResourceBuffer());
    bindTarget(false);
    try
    {
        checkErrors();
    }
    catch (ExceptionShaderRenderError& e)
    {
        errors.push_back("Failed to read color buffer back.");
        errors.insert(std::end(errors), std::begin(e.errorLog()), std::end(e.errorLog()));
        throw ExceptionShaderRenderError(errorType, errors);
    }

    // Save using the handler.
    if (!_imageHandler->saveImage(filePath, image, true))
    {
        errors.push_back("Failed to save to file:" + filePath.asString());
        throw ExceptionShaderRenderError(errorType, errors);
    }
}

void GlslRenderer::checkErrors()
{
    StringVec errors;

    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        errors.push_back("OpenGL error: " + std::to_string(error));
    }
    if (errors.size())
    {
        throw ExceptionShaderRenderError("OpenGL context error.", errors);
    }
}

} // namespace MaterialX
