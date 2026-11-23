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

#include <unordered_map>
#include <utility>
#include <vector>

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

    /// Render a fullscreen quad covering the given UV range. Used by
    /// the texture baker to render a shader's output into texture
    /// space. Mirrors GlslRenderer::renderTextureSpace; uvMin/uvMax
    /// default to (0,0)/(1,1) on the baker side, which is the only
    /// case the current implementation hits, so we just route to the
    /// existing fullscreen-triangle path. Non-default UV ranges are
    /// not yet honoured.
    void renderTextureSpace(const Vector2& uvMin, const Vector2& uvMax);

    ImagePtr captureImage(ImagePtr image = nullptr) override;

    /// Background color used for the framebuffer clear before render().
    void setScreenColor(const Color4& color) { _screenColor = color; }
    const Color4& getScreenColor() const { return _screenColor; }

    /// Restrict draw iteration to meshes whose Mesh::getName() is in
    /// this set. Empty set (default) renders every mesh exposed by the
    /// geometry handler. Used by the shaderball convention to render
    /// the Preview_Mesh under the user material and Calibration_Mesh
    /// under a separate reference material in two passes.
    void setActiveMeshes(const StringSet& names) { _activeMeshes = names; }

    /// Skip the framebuffer clear at the top of render(). Used for
    /// multi-pass renders where pass N>0 must overwrite only the pixels
    /// it actually rasterises. Defaults to true.
    void setClearOnRender(bool clear) { _clearOnRender = clear; }

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
    bool bindImage(const std::string& uniformName, ImagePtr image,
                   const ImageSamplingProperties* properties = nullptr);

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
    /// HlslMaterial::patchArrayMember which walks the reflected struct
    /// type for u_lightData to compute member offsets - D3D reflection
    /// doesn't expose array element members by composed name, so a
    /// composed-name lookup like `u_lightData[0].position` returns
    /// NOT_FOUND. Inputs without a matching reflected member are
    /// skipped silently.
    void bindLightSourcesFromLightHandler();

    /// Walk the most-recently-bound shader's pixel-stage PUBLIC_UNIFORMS
    /// for scalar/vector members and write each value into the cbuffer
    /// that owns it. The HLSL generator emits PUBLIC_UNIFORMS values as
    /// metadata in the cbuffer struct, but the GPU buffer itself is
    /// zero-initialised by D3D11 - without this pass, surface
    /// parameters like base_color, opacity, emission stay zero in the
    /// shader and the surface evaluates to black. FILENAME-typed
    /// uniforms are skipped here; they're handled by
    /// bindFileTexturesFromImageHandler.
    void bindMaterialUniformsFromShader();

    /// Build and stash a fullscreen-triangle vertex buffer on first use.
    void ensureFullscreenGeometry();

    /// Build and stash interleaved (POSITION + NORMAL + TANGENT + UV)
    /// vertex buffers per mesh and index buffers per partition for every
    /// mesh/partition the geometry handler exposes. Mirrors GlslRenderer's
    /// "iterate all meshes, draw all partitions" loop. Cached by the
    /// (Mesh*, Partition*) identity vector; a different scene-graph
    /// invalidates the cache and re-uploads. Returns true if at least one
    /// drawable partition is available.
    bool ensureMeshGeometry();

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
    StringSet _activeMeshes;          // empty = render every mesh
    bool _clearOnRender = true;

    ID3D11Buffer*           _fullscreenVbuf = nullptr;
    ID3D11RasterizerState*  _rasterState = nullptr;

    // Mesh geometry cache. One MeshDraw per partition. Cached vertex
    // buffers are shared across partitions of the same mesh via the
    // _meshVbufCache map; partitions own their own index buffer. The
    // _meshCacheKey vector is the (Mesh*, Partition*) identity sequence
    // captured on the most recent build; if a later render finds a
    // different sequence we tear down and rebuild.
    struct MeshDraw {
        ID3D11Buffer* vbuf = nullptr;        // borrowed from _meshVbufCache
        ID3D11Buffer* ibuf = nullptr;        // owned
        unsigned int  indexCount = 0;
    };
    std::vector<std::pair<void*, void*>>           _meshCacheKey;
    std::unordered_map<void*, ID3D11Buffer*>       _meshVbufCache;
    std::vector<MeshDraw>                          _meshDraws;
};

MATERIALX_NAMESPACE_END

#endif
