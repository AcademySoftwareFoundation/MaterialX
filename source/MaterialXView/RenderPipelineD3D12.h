//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef RENDER_PIPELINE_D3D12_H
#define RENDER_PIPELINE_D3D12_H

#include <MaterialXView/RenderPipeline.h>

#include <MaterialXRenderHlsl/HlslD3D12Framebuffer.h>
#include <MaterialXRenderHlsl/HlslD3D12GeometryCache.h>
#include <MaterialXRenderHlsl/HlslD3D12ShaderMaterial.h>

class Viewer;
using D3D12RenderPipelinePtr = std::shared_ptr<class D3D12RenderPipeline>;

// Render pipeline that draws the scene with Direct3D 12 into an off-screen
// framebuffer, then copies the image into the OpenGL back buffer of the
// viewer window, where NanoGUI draws the user interface on top.
class D3D12RenderPipeline : public RenderPipeline
{
  public:
    D3D12RenderPipeline(Viewer* viewerPtr);
    ~D3D12RenderPipeline();

    static D3D12RenderPipelinePtr create(Viewer* viewer)
    {
        return std::make_shared<D3D12RenderPipeline>(viewer);
    }

    void initialize(void* device, void* command_queue) override;

    void initFramebuffer(int width, int height,
                         void* color_texture) override;
    void resizeFramebuffer(int width, int height,
                           void* color_texture) override;

    mx::ImageHandlerPtr createImageHandler() override;
    mx::MaterialPtr     createMaterial() override;
    void updateAlbedoTable(int tableSize) override;
    void updatePrefilteredMap() override;
    std::shared_ptr<void> createTextureBaker(unsigned int width,
                                             unsigned int height,
                                             mx::Image::BaseType baseType) override;
    void renderFrame(void* color_texture, int shadowMapSize, const char* dirLightNodeCat) override;
    void bakeTextures() override;
    mx::ImagePtr getFrameImage() override;

  protected:
    mx::ImagePtr getShadowMap(int shadowMapSize) override;

  private:
    // Create a material for a supporting shader, or return null and report
    // the error.
    mx::HlslD3D12ShaderMaterialPtr createSupportMaterial(mx::ShaderPtr hwShader);

    // Draw `material` over a screen-space quad into `framebuffer`, cleared
    // to `clearColor`, in a frame of its own, and read the result back.
    mx::ImagePtr renderQuad(mx::HlslD3D12ShaderMaterialPtr material,
                            mx::HlslD3D12FramebufferPtr framebuffer,
                            const mx::Color4& clearColor);

    // Copy a top-down RGBA8 image into the OpenGL back buffer.
    void presentToWindow(mx::ImagePtr image);

    mx::HlslD3D12TextureHandlerPtr getTextureHandler() const;

    mx::HlslD3D12ContextPtr _context;
    mx::HlslD3D12GeometryCachePtr _geometryCache;
    mx::HlslD3D12FramebufferPtr _framebuffer;
    mx::ImagePtr _frameImage;

    unsigned int _glTexture;
    unsigned int _glFramebuffer;
    int _glTextureWidth;
    int _glTextureHeight;
};

#endif // RENDER_PIPELINE_D3D12_H
