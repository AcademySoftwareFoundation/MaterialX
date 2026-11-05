//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLD3D12RENDERER_H
#define MATERIALX_HLSLD3D12RENDERER_H

/// @file
/// MaterialX ShaderRenderer subclass driving the HLSL pipeline on D3D12.

#include <MaterialXRenderHlsl/Export.h>
#include <MaterialXRenderHlsl/HlslD3D12Context.h>
#include <MaterialXRenderHlsl/HlslD3D12Framebuffer.h>
#include <MaterialXRenderHlsl/HlslD3D12GeometryCache.h>
#include <MaterialXRenderHlsl/HlslD3D12Material.h>
#include <MaterialXRenderHlsl/HlslD3D12TextureHandler.h>
#include <MaterialXRenderHlsl/HlslProgram.h>

#include <MaterialXRender/ImageHandler.h>
#include <MaterialXRender/ShaderRenderer.h>

MATERIALX_NAMESPACE_BEGIN

class HlslD3D12Renderer;
using HlslD3D12RendererPtr = shared_ptr<class HlslD3D12Renderer>;

/// @class HlslD3D12Renderer
/// ShaderRenderer for the HLSL backend on Direct3D 12; the D3D12
/// counterpart of HlslRenderer, with the same public workflow.
///
/// createProgram() compiles the generated shader with DXC at shader model
/// 6.0 when dxcompiler.dll is available, and with FXC at shader model 5.0
/// otherwise. render() binds material, camera, texture and lighting
/// uniforms from the attached handlers, draws every mesh of the geometry
/// handler (or a fullscreen triangle when there is none) into an
/// off-screen framebuffer, and waits for completion. captureImage() reads
/// the framebuffer back to the CPU.
class MX_RENDERHLSL_API HlslD3D12Renderer : public ShaderRenderer
{
  public:
    static HlslD3D12RendererPtr create(unsigned int width = 512,
                                       unsigned int height = 512,
                                       Image::BaseType baseType = Image::BaseType::UINT8);

    ~HlslD3D12Renderer() override;

    /// Build an image handler that uploads through this renderer's device.
    /// Used by the TextureBaker through the Renderer::createImageHandler hook.
    ImageHandlerPtr createImageHandler(ImageLoaderPtr imageLoader);

    /// Use the WARP software adapter instead of a hardware adapter. Must be
    /// called before initialize().
    void setPreferWarp(bool preferWarp) { _preferWarp = preferWarp; }
    bool getPreferWarp() const { return _preferWarp; }

    /// Override the compiler backend and shader model used by createProgram.
    void setCompilerBackend(HlslCompilerBackend backend, const std::string& shaderModel);
    HlslCompilerBackend getCompilerBackend() const { return _backend; }
    const std::string& getShaderModel() const { return _shaderModel; }

    void initialize(RenderContextHandle = nullptr) override;
    void createProgram(ShaderPtr shader) override;
    void createProgram(const StageMap& stages) override;
    void validateInputs() override;
    void setSize(unsigned int width, unsigned int height) override;
    void render() override;

    /// Render a fullscreen triangle whose texture coordinates span uvMin to
    /// uvMax across the viewport. Used by the texture baker.
    void renderTextureSpace(const Vector2& uvMin, const Vector2& uvMax);

    /// Read the framebuffer back. When `image` matches the framebuffer size
    /// and format it is filled and returned.
    ImagePtr captureImage(ImagePtr image = nullptr) override;

    /// Background color used for the framebuffer clear before render().
    void setScreenColor(const Color4& color) { _screenColor = color; }
    const Color4& getScreenColor() const { return _screenColor; }

    /// Restrict drawing to meshes whose name is in this set. An empty set
    /// (the default) draws every mesh of the geometry handler.
    void setActiveMeshes(const StringSet& names) { _activeMeshes = names; }

    /// Skip the framebuffer clear at the top of render(), for multi-pass
    /// renders. Defaults to true.
    void setClearOnRender(bool clear) { _clearOnRender = clear; }

    /// Fixed-function state used for draws.
    void setRenderState(const HlslD3D12RenderState& state) { _renderState = state; }
    const HlslD3D12RenderState& getRenderState() const { return _renderState; }

    /// Upload `image` and bind it to the texture and sampler of the named
    /// SamplerTexture2D uniform of the pixel stage. Returns true when at
    /// least one of the two registers was found.
    bool bindImage(const std::string& uniformName, ImagePtr image,
                   const ImageSamplingProperties* properties = nullptr);

    /// Borrowed accessors.
    HlslD3D12ContextPtr getContext() const { return _context; }
    HlslD3D12FramebufferPtr getFramebuffer() const { return _framebuffer; }
    HlslProgramPtr getProgram() const { return _program; }
    HlslD3D12MaterialPtr getMaterial() const { return _material; }
    HlslD3D12TextureHandlerPtr getTextureHandler() const { return _textureHandler; }

  protected:
    HlslD3D12Renderer(unsigned int width, unsigned int height, Image::BaseType baseType);

    /// Write material, camera and lighting uniforms and bind file and
    /// environment textures from the attached handlers.
    void bindUniformsFromHandlers();

    /// Record draws of `meshes` (every partition) with the current material.
    void renderMeshes(const std::vector<MeshPtr>& meshes);

  private:
    HlslD3D12ContextPtr _context;
    HlslD3D12FramebufferPtr _framebuffer;
    HlslProgramPtr _program;
    HlslD3D12MaterialPtr _material;
    HlslD3D12TextureHandlerPtr _textureHandler;
    ShaderPtr _shader;

    bool _preferWarp = false;
    HlslCompilerBackend _backend;
    std::string _shaderModel;

    Color4 _screenColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
    StringSet _activeMeshes;
    bool _clearOnRender = true;
    HlslD3D12RenderState _renderState;

    HlslD3D12GeometryCachePtr _geometry;
    MeshPtr _fullscreenMesh;
    Vector2 _fullscreenUvMin = Vector2(0.0f, 0.0f);
    Vector2 _fullscreenUvMax = Vector2(1.0f, 1.0f);
};

MATERIALX_NAMESPACE_END

#endif
