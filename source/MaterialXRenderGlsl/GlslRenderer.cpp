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
    _res(res),
    _initialized(false)
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

    // Clean up frame buffer
    _frameBuffer = nullptr;

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
        if (!_window->initialize(const_cast<char *>(windowName), _res, _res, nullptr))
        {
            errors.push_back("Failed to create window for testing.");
            throw ExceptionShaderRenderError(errorType, errors);
        }

        // Create offscreen context
        _context = GLUtilityContext::create(_window->windowWrapper(), nullptr);
        if (!_context)
        {
            errors.push_back("Failed to create OpenGL context for testing.");
            throw ExceptionShaderRenderError(errorType, errors);
        }

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

            _frameBuffer = GLFramebuffer::create(_res, _res, 4, Image::BaseType::UINT8);

            _initialized = true;
        }
    }
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

void GlslRenderer::renderTextureSpace()
{
    _program->bind();
    _program->bindTextures(_imageHandler);

    _frameBuffer->bind();
    drawScreenSpaceQuad();
    _frameBuffer->unbind();

    _program->unbind();
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

    _viewHandler->viewMatrix = ViewHandler::createViewMatrix(eye, center, up);
    _viewHandler->projectionMatrix = ViewHandler::createPerspectiveMatrix(-fW, fW, -fH, fH, nearDist, farDist);
    _viewHandler->worldMatrix = Matrix44::createTranslation(modelTranslation) *
                                Matrix44::createScale(Vector3(objectScale * meshFit));

    Matrix44 invView = _viewHandler->viewMatrix.getInverse();
    _viewHandler->viewDirection = { invView[2][0], invView[2][1], invView[2][2] };
    _viewHandler->viewPosition = { invView[3][0], invView[3][1], invView[3][2] };
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
    _frameBuffer->bind();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
        _frameBuffer->unbind();
        throw;
    }

    // Unset target
    _frameBuffer->unbind();
}

void GlslRenderer::save(const FilePath& filePath)
{
    StringVec errors;
    const string errorType("GLSL image save error.");

    if (!_imageHandler)
    {
        errors.push_back("No image handler specified.");
        throw ExceptionShaderRenderError(errorType, errors);
    }

    ImagePtr image = _frameBuffer->createColorImage();

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

void GlslRenderer::drawScreenSpaceQuad()
{
    const float QUAD_VERTICES[] =
    {
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f, // position, texcoord
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f
    };
    const unsigned int QUAD_INDICES[] =
    {
        0, 1, 3,
        1, 2, 3
    };
    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTICES), QUAD_VERTICES, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (3 * sizeof(float)));

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(QUAD_INDICES), QUAD_INDICES, GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

} // namespace MaterialX
