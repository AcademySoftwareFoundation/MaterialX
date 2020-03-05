//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_GLSLRENDERER_H
#define MATERIALX_GLSLRENDERER_H

/// @file
/// GLSL code renderer

#include <MaterialXRenderGlsl/GLFramebuffer.h>
#include <MaterialXRenderGlsl/GlslProgram.h>

#include <MaterialXRender/ShaderRenderer.h>

namespace MaterialX
{

using GLUtilityContextPtr = std::shared_ptr<class GLUtilityContext>;
using SimpleWindowPtr = std::shared_ptr<class SimpleWindow>;

/// Shared pointer to a GlslRenderer
using GlslRendererPtr = std::shared_ptr<class GlslRenderer>;

/// @class GlslRenderer
/// Helper class for rendering generated GLSL code to produce images.
///
/// There are two main interfaces which can be used. One which takes in a HwShader and one which
/// allows for explicit setting of shader stage code.
///
/// The main services provided are:
///     - Validation: All shader stages are compiled and atteched to a GLSL shader program.
///     - Introspection: The compiled shader program is examined for uniforms and attributes.
///     - Binding: Uniforms and attributes which match the predefined variables generated the GLSL code generator
///       will have values assigned to this. This includes matrices, attribute streams, and textures.
///     - Rendering: The program with bound inputs will be used to drawing geometry to an offscreen buffer.
///     An interface is provided to save this offscreen buffer to disk using an externally defined image handler.
///
class GlslRenderer : public ShaderRenderer
{
  public:
    /// Create a GLSL renderer instance
    static GlslRendererPtr create(unsigned int res = 512);

    /// Destructor
    virtual ~GlslRenderer();

    /// @name Setup
    /// @{

    /// Internal initialization of stages and OpenGL constructs
    /// required for program validation and rendering.
    /// An exception is thrown on failure.
    /// The exception will contain a list of initialization errors.
    void initialize() override;

    /// @}
    /// @name Rendering
    /// @{

    /// Create GLSL program based on an input shader
    /// @param shader Input HwShader
    void createProgram(ShaderPtr shader) override;

    /// Create GLSL program based on shader stage source code.
    /// @param stages Map of name and source code for the shader stages.
    void createProgram(const StageMap& stages) override;

    /// Validate inputs for the program
    void validateInputs() override;

    /// Render the current program to an offscreen buffer.
    void render() override;

    /// Render the current program in texture space to an off-screen buffer.
    void renderTextureSpace();

    /// @}
    /// @name Utilities
    /// @{

    /// Save the current contents the offscreen hardware buffer to disk.
    /// @param filePath Name of file to save rendered image to.
    void save(const FilePath& filePath) override;

    /// Return the GL frame buffer.
    GLFrameBufferPtr getFrameBuffer() const
    {
        return _frameBuffer;
    }

    /// Return the GLSL program.
    GlslProgramPtr getProgram()
    {
        return _program;
    }

    /// Submit geometry for a screen-space quad.
    static void drawScreenSpaceQuad();

    /// @}

  protected:
    GlslRenderer(unsigned int res);

    void updateViewInformation(const Vector3& eye,
                               const Vector3& center,
                               const Vector3& up,
                               float viewAngle,
                               float nearDist,
                               float farDist,
                               float objectScale);

  private:
    void checkErrors();

  private:
    GlslProgramPtr _program;

    GLFrameBufferPtr _frameBuffer;
    unsigned int _res;

    bool _initialized;

    SimpleWindowPtr _window;
    GLUtilityContextPtr _context;
};

} // namespace MaterialX

#endif
