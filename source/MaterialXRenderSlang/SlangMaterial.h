//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SLANGMATERIAL_H
#define MATERIALX_SLANGMATERIAL_H

/// @file
/// Slang code material - used in MaterialXView

#include <MaterialXRenderSlang/Export.h>
#include <MaterialXRenderSlang/SlangProgram.h>

#include <MaterialXRender/ShaderMaterial.h>

#include <vector>
#include <unordered_map>

MATERIALX_NAMESPACE_BEGIN

using SlangFramebufferPtr = std::shared_ptr<class SlangFramebuffer>;
using SlangProgramPtr = std::shared_ptr<class SlangProgram>;
using SlangContextPtr = std::shared_ptr<class SlangContext>;
using SlangMaterialPtr = std::shared_ptr<class SlangMaterial>;

class SlangMaterial : public ShaderMaterial
{
  public:
    static SlangMaterialPtr create(SlangContextPtr context);

    SlangMaterial(SlangContextPtr context);
    virtual ~SlangMaterial();

    /// Load shader source from file.
    bool loadSource(const FilePath& vertexShaderFile,
                    const FilePath& pixelShaderFile,
                    bool hasTransparency) override;

    /// Generate a shader from our currently stored element and
    /// the given generator context.
    bool generateShader(GenContext& context) override;

    /// Copies shader and API specific generated program from ShaderMaterial to this one.
    void copyShader(MaterialPtr ShaderMaterial) override;

    /// Return the underlying Slang program.
    SlangProgramPtr getProgram() const
    {
        return _glProgram;
    }

    /// Generate a shader from the given hardware shader.
    bool generateShader(ShaderPtr hwShader) override;

    /// Bind shader
    bool bindShader() const override
    {
        return true;
    }

    /// Bind viewing information for this ShaderMaterial.
    void bindViewInformation(CameraPtr camera) override;

    /// Bind all images for this ShaderMaterial.
    void bindImages(ImageHandlerPtr imageHandler,
                    const FileSearchPath& searchPath,
                    bool enableMipmaps = true) override;

    /// Unbbind all images for this ShaderMaterial.
    void unbindImages(ImageHandlerPtr imageHandler) override;

    /// Bind a single image.
    ImagePtr bindImage(const FilePath& filePath,
                       const std::string& uniformName,
                       ImageHandlerPtr imageHandler,
                       const ImageSamplingProperties& samplingProperties) override;

    /// Bind lights to shader.
    void bindLighting(LightHandlerPtr lightHandler,
                      ImageHandlerPtr imageHandler,
                      const ShadowState& shadowState) override;

    /// Bind the given mesh to this material.
    void bindMesh(MeshPtr mesh) override;

    /// Bind a mesh partition to this material.
    bool bindPartition(MeshPartitionPtr part) const override;

    /// Draw the given mesh partition.
    void drawPartition(MeshPartitionPtr part) const override;

    /// Unbind all geometry from this material.
    void unbindGeometry() override;

    /// Return the block of public uniforms for this material.
    VariableBlock* getPublicUniforms() const override;

    /// Find a public uniform from its MaterialX path.
    ShaderPort* findUniform(const std::string& path) const override;

    /// Modify the value of the uniform with the given path.
    void modifyUniform(const std::string& path,
                       ConstValuePtr value,
                       std::string valueString = EMPTY_STRING) override;

    void prepareUsedResources(CameraPtr cam,
                              GeometryHandlerPtr geometryHandler,
                              ImageHandlerPtr imageHandler,
                              LightHandlerPtr lightHandler);

    void bindEncoder(rhi::IRenderPassEncoder* passEncoder,
                     rhi::RenderState* renderState)
    {
        _passEncoder = passEncoder;
        _renderState = renderState;
    }

    void unbindEncoder()
    {
        _passEncoder = nullptr;
        _renderState = nullptr;
    }

  protected:
    void clearShader() override;

    SlangContextPtr _context;
    SlangProgramPtr _glProgram;

    rhi::IRenderPassEncoder* _passEncoder = nullptr;
    rhi::RenderState* _renderState = nullptr;
};

MATERIALX_NAMESPACE_END

#endif
