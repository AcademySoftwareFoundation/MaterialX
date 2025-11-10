//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SLANGRENDERER_H
#define MATERIALX_SLANGRENDERER_H

/// @file
/// Slang code renderer

#include <MaterialXRenderSlang/Export.h>

#include <MaterialXRenderSlang/SlangProgram.h>

#include <MaterialXRender/ImageHandler.h>
#include <MaterialXRender/ShaderRenderer.h>

MATERIALX_NAMESPACE_BEGIN

using SlangFramebufferPtr = std::shared_ptr<class SlangFramebuffer>;
using SlangContextPtr = std::shared_ptr<class SlangContext>;
using SimpleWindowPtr = std::shared_ptr<class SimpleWindow>;

// Shared pointer to an SlangRenderer
using SlangRendererPtr = std::shared_ptr<class SlangRenderer>;

/// @class SlangRenderer
/// Helper class for rendering generated Slang code to produce images.
///
class MX_RENDERSLANG_API SlangRenderer : public ShaderRenderer
{
  public:
    /// Create an Slang renderer instance
    static SlangRendererPtr create(unsigned int width = 512, unsigned int height = 512, Image::BaseType baseType = Image::BaseType::UINT8);

    /// Destructor
    virtual ~SlangRenderer();

    void reset();

    /// Create a texture handler for OpenGL textures
    ImageHandlerPtr createImageHandler(ImageLoaderPtr imageLoader);

    /// @name Setup
    /// @{

    /// Internal initialization required for program validation and rendering.
    /// An exception is thrown on failure.
    /// The exception will contain a list of initialization errors.
    void initialize(RenderContextHandle renderContextHandle = nullptr) override;

    /// @}
    /// @name Rendering
    /// @{

    /// Create Slang program based on an input shader
    ///
    /// @param shader Input shader
    void createProgram(ShaderPtr shader) override;

    /// Create Slang program based on shader stage source code.
    /// @param stages Map of name and source code for the shader stages.
    void createProgram(const StageMap& stages) override;

    SlangProgramPtr getProgram() const { return _program; }

    /// Validate inputs for the compiled Slang program.
    /// Note: Currently no validation has been implemented.
    void validateInputs() override;

    /// Set the size for rendered image
    void setSize(unsigned int width, unsigned int height) override;

    /// Render Slang program to disk.
    void render() override;

    /// @}
    /// @name Utilities
    /// @{

    /// Capture the current rendered output as an image.
    ImagePtr captureImage(ImagePtr image = nullptr) override;

    /// Set the screen background color.
    void setScreenColor(const Color3& screenColor)
    {
        _screenColor = screenColor;
    }

    /// Return the screen background color.
    Color3 getScreenColor() const
    {
        return _screenColor;
    }

    /// Return the Slang frame buffer.
    SlangFramebufferPtr getFramebuffer() const
    {
        return _framebuffer;
    }

    /// Render the current program in texture space to an off-screen buffer.
    void renderTextureSpace(const Vector2& uvMin, const Vector2& uvMax);

  protected:
    /// Constructor
    SlangRenderer(unsigned int width, unsigned int height, Image::BaseType baseType);

    void createFrameBuffer(bool encodeSrgb);

  private:
    // SlangProgramPtr _program;

    bool _initialized;

    SlangContextPtr _context;
    SimpleWindowPtr _window;
    SlangProgramPtr _program;
    SlangFramebufferPtr _framebuffer;

    Color3 _screenColor;
};

MATERIALX_NAMESPACE_END

#endif
