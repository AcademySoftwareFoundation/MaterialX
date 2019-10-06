//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_GLSLRENDERER_H
#define MATERIALX_GLSLRENDERER_H

/// @file
/// GLSL code renderer

#include <MaterialXRenderGlsl/GlslProgram.h>

#include <MaterialXRender/ImageHandler.h>
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
    void createProgram(const ShaderPtr shader) override;

    /// Create GLSL program based on shader stage source code.
    /// @param stages Map of name and source code for the shader stages.
    void createProgram(const StageMap& stages) override;

    /// Validate inputs for the program
    void validateInputs() override;

    /// Render the current program to an offscreen buffer.
    void render() override;

    /// Render the current program in texture space to an off-screen buffer.
    /// @param encodeSrgb If true, then the off-screen buffer will be encoded
    ///    as sRGB; otherwise, no encoding is performed.
    void renderTextureSpace(bool encodeSrgb);

    /// @}
    /// @name Utilities
    /// @{

    /// Save the current contents the offscreen hardware buffer to disk.
    /// @param filePath Name of file to save rendered image to.
    /// @param floatingPoint Format of output image is floating point.
    void save(const FilePath& filePath, bool floatingPoint) override;

    /// Return the GLSL program wrapper class
    MaterialX::GlslProgramPtr program()
    {
        return _program;
    }

    /// @}

  protected:
    /// Constructor
    GlslRenderer(unsigned int res);

    /// @name Target handling
    /// @{

    /// Create a offscreen target used for rendering.
    bool createTarget();
    /// Delete any created offscreen target.
    void deleteTarget();
    /// Bind or unbind any created offscree target.
    bool bindTarget(bool bind);

    /// @}
    /// @name Program bindings
    /// @{

    /// Update viewing information
    /// @param eye Eye position
    /// @param center Center of focus 
    /// @param up Up vector
    /// @param viewAngle Viewing angle in degrees
    /// @param nearDist Distance to near plane
    /// @param farDist Distance to far plane
    /// @param objectScale Scale to apply to geometry
    void updateViewInformation(const Vector3& eye,
                               const Vector3& center,
                               const Vector3& up,
                               float viewAngle,
                               float nearDist,
                               float farDist,
                               float objectScale);

  private:
    /// Utility to check for OpenGL context errors.
    /// Will throw an ExceptionShaderRenderError exception which will list of the errors found
    /// if any errors encountered.
    void checkErrors();

    /// GLSL program.
    GlslProgramPtr _program;

    /// Hardware color target (texture)
    unsigned int _colorTarget;

    /// Hardware depth target (texture)
    unsigned int _depthTarget;

    /// Hardware frame buffer object
    unsigned int _frameBuffer;

    /// Width of the frame buffer / targets to use.
    unsigned int _frameBufferWidth;
    /// Height of the frame buffer / targets to use.
    unsigned int _frameBufferHeight;

    /// Flag to indicate if renderer has been initialized properly.
    bool _initialized;

    /// Dummy window for OpenGL usage.
    SimpleWindowPtr _window;
    /// Dummy OpenGL context for OpenGL usage
    GLUtilityContextPtr _context;
};

} // namespace MaterialX

#endif
