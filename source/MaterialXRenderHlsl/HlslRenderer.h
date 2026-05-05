//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLRENDERER_H
#define MATERIALX_HLSLRENDERER_H

/// @file
/// MaterialX ShaderRenderer subclass driving the HLSL pipeline.

#include <MaterialXRenderHlsl/Export.h>
#include <MaterialXRenderHlsl/HlslContext.h>
#include <MaterialXRenderHlsl/HlslFramebuffer.h>
#include <MaterialXRenderHlsl/HlslMaterial.h>
#include <MaterialXRenderHlsl/HlslProgram.h>
#include <MaterialXRenderHlsl/HlslTextureHandler.h>

#include <MaterialXRender/ImageHandler.h>
#include <MaterialXRender/ShaderRenderer.h>

struct ID3D11Buffer;
struct ID3D11RasterizerState;

MATERIALX_NAMESPACE_BEGIN

class HlslRenderer;
using HlslRendererPtr = shared_ptr<class HlslRenderer>;

/// @class HlslRenderer
/// MVP ShaderRenderer for the HLSL backend. Owns a D3D11 device, an
/// off-screen framebuffer, an HlslProgram + HlslMaterial built from a
/// MaterialX-generated shader, and an HlslTextureHandler for image
/// uploads. render() binds the material, writes the camera's world /
/// view-projection matrices into the reflected vertexCB offsets if a
/// Camera has been set, and draws a fullscreen triangle. captureImage()
/// returns the GPU framebuffer as a MaterialX Image.
///
/// Geometry-driven rendering (binding a Mesh's vertex/index buffers
/// instead of the fullscreen triangle), light-handler binding into
/// LightData cbuffers, and image-handler-driven texture binding into
/// the t#/s# slots are out of scope here; this class is the seam those
/// integrations attach to.
class MX_RENDERHLSL_API HlslRenderer : public ShaderRenderer
{
  public:
    static HlslRendererPtr create(unsigned int width = 512,
                                  unsigned int height = 512,
                                  Image::BaseType baseType = Image::BaseType::UINT8);

    ~HlslRenderer() override;

    /// Build a texture handler bound to this renderer's D3D11 device.
    /// Mirrors GlslRenderer::createImageHandler / MslRenderer::create
    /// ImageHandler - returns an ImageHandler subclass (HlslTexture
    /// Handler) that doubles as both the image loader and the GPU
    /// upload cache. The TextureBaker base calls this through the
    /// generic Renderer::createImageHandler hook.
    ImageHandlerPtr createImageHandler(ImageLoaderPtr imageLoader)
    {
        if (!_context)
            initialize();
        return HlslTextureHandler::create(_context, imageLoader);
    }

    void initialize(RenderContextHandle = nullptr) override;
    void createProgram(ShaderPtr shader) override;
    void createProgram(const StageMap& stages) override;
    void validateInputs() override;
    void setSize(unsigned int width, unsigned int height) override;
    void render() override;
    ImagePtr captureImage(ImagePtr image = nullptr) override;

    /// Background color used for the framebuffer clear before render().
    void setScreenColor(const Color4& color) { _screenColor = color; }
    const Color4& getScreenColor() const { return _screenColor; }

    /// Upload `image` via the renderer's HlslTextureHandler and bind it
    /// to the texture/sampler slots of the named SamplerTexture2D handle
    /// in the pixel stage. The HLSL generator emits each file-texture
    /// input as a `SamplerTexture2D <uniformName>` global; D3D shader
    /// reflection reports the struct's members as separate bindings
    /// named `<uniformName>.tex` (texture) and `<uniformName>.samp`
    /// (sampler). bindImage walks the material's reflected pixel
    /// bindings, finds both, and routes the SRV / sampler from the
    /// uploaded image into the matching t# / s# slots. Returns true
    /// when at least one slot was bound.
    bool bindImage(const std::string& uniformName, ImagePtr image);

    /// Borrowed accessors.
    HlslContextPtr        getContext()        const { return _context; }
    HlslFramebufferPtr    getFramebuffer()    const { return _framebuffer; }
    HlslProgramPtr        getProgram()        const { return _program; }
    HlslMaterialPtr       getMaterial()       const { return _material; }
    HlslTextureHandlerPtr getTextureHandler() const { return _textureHandler; }

  protected:
    HlslRenderer(unsigned int width, unsigned int height, Image::BaseType baseType);

    /// Walk the most-recently-bound shader's pixel-stage PUBLIC_UNIFORMS
    /// for FILENAME-typed uniforms and auto-bind each one through the
    /// attached ImageHandler. Skips lighting textures (env radiance /
    /// irradiance), which need explicit setup. No-op when either the
    /// image handler or the shader is unavailable.
    void bindFileTexturesFromImageHandler();

    /// Bind the environment radiance and irradiance images to their
    /// SamplerTexture2D handles. Pulls real maps from the LightHandler
    /// when one is attached and indirect lighting is enabled; falls back
    /// to ImageHandler::getZeroImage() otherwise so the shader never
    /// samples a null SRV.
    void bindEnvironmentImagesFromLightHandler();

    /// Write the scalar environment uniforms (env matrix, light
    /// intensity, sample count, mip count, refraction two-sidedness)
    /// and u_numActiveLightSources from the LightHandler / ImageHandler
    /// into the pixel cbuffer at the offsets reported by reflection.
    void bindLightingScalarsFromHandlers();

    /// Walk LightHandler::getLightSources(), write each light's type id
    /// (from computeLightIdMap) and each MaterialX input value into the
    /// matching member of the corresponding u_lightData[i] struct. Uses
    /// D3D shader reflection's indexed-name lookup
    /// ("u_lightData[0].position" etc.) to find each member's byte
    /// offset, so the layout doesn't need to be hard-coded. Inputs that
    /// don't have a matching member in the reflected layout are skipped
    /// silently.
    void bindLightSourcesFromLightHandler();

    /// Build and stash a fullscreen-triangle vertex buffer on first use.
    void ensureFullscreenGeometry();

    /// Build and stash interleaved (POSITION + NORMAL + TANGENT) vertex
    /// and index buffers from the geometry handler's first mesh. Cached
    /// by mesh pointer so subsequent renders reuse the GPU buffers.
    /// Returns the partition index count, or 0 if no mesh is available.
    unsigned int ensureMeshGeometry();

    /// Copy the current camera's world and view-projection matrices into
    /// the vertexCB at the offsets reported by reflection. Silently does
    /// nothing if the cbuffer or members aren't present.
    void bindCameraToVertexCbuffer();

  private:
    HlslContextPtr        _context;
    HlslFramebufferPtr    _framebuffer;
    HlslProgramPtr        _program;
    HlslMaterialPtr       _material;
    HlslTextureHandlerPtr _textureHandler;

    // The MaterialX shader currently bound. Cached so render() can walk
    // its PUBLIC_UNIFORMS for FILENAME-typed inputs and auto-bind them
    // through the attached ImageHandler.
    ShaderPtr _shader;

    Color4 _screenColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);

    ID3D11Buffer*           _fullscreenVbuf = nullptr;
    ID3D11RasterizerState*  _rasterState = nullptr;

    // Mesh geometry cache. _meshKey is the void* identity of the most
    // recently bound MeshPartition; if the geometry handler returns a
    // different partition we discard the cached buffers and re-upload.
    void*           _meshKey = nullptr;
    ID3D11Buffer*   _meshVbuf = nullptr;
    ID3D11Buffer*   _meshIbuf = nullptr;
    unsigned int    _meshIndexCount = 0;
};

MATERIALX_NAMESPACE_END

#endif
