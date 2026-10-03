//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLD3D12SHADERMATERIAL_H
#define MATERIALX_HLSLD3D12SHADERMATERIAL_H

/// @file
/// ShaderMaterial for HLSL generation and D3D12 rendering.

#include <MaterialXRenderHlsl/Export.h>
#include <MaterialXRenderHlsl/HlslD3D12GeometryCache.h>
#include <MaterialXRenderHlsl/HlslD3D12Material.h>
#include <MaterialXRenderHlsl/HlslD3D12TextureHandler.h>

#include <MaterialXRender/ShaderMaterial.h>

MATERIALX_NAMESPACE_BEGIN

using HlslD3D12ShaderMaterialPtr = std::shared_ptr<class HlslD3D12ShaderMaterial>;

/// @class HlslD3D12ShaderMaterial
/// ShaderMaterial for HLSL generation and D3D12 rendering; the D3D12
/// counterpart of GlslMaterial, used by the D3D12 pipeline of the viewer.
///
/// Like GlslMaterial, it follows the current device state rather than
/// taking it as arguments: drawPartition() records onto the frame command
/// list of the context, so it must be called between
/// HlslD3D12Context::beginFrame() and endFrame() after a framebuffer has
/// been bound, and the pipeline state follows the bound framebuffer formats
/// and HlslD3D12Context::getRenderState().
///
/// Public uniform values are kept on the generated shader, as in
/// GlslMaterial, and written to the constant buffers at each draw. Images
/// must be bound through an HlslD3D12TextureHandler created on the same
/// context. Uploaded textures stay resident: unbindImages() only forgets
/// the images of the last bindImages() call, since a recorded frame may
/// still reference them.
class MX_RENDERHLSL_API HlslD3D12ShaderMaterial : public ShaderMaterial
{
  public:
    HlslD3D12ShaderMaterial(HlslD3D12ContextPtr context, HlslD3D12GeometryCachePtr geometryCache);
    ~HlslD3D12ShaderMaterial() override;

    static HlslD3D12ShaderMaterialPtr create(HlslD3D12ContextPtr context, HlslD3D12GeometryCachePtr geometryCache)
    {
        return std::make_shared<HlslD3D12ShaderMaterial>(context, geometryCache);
    }

    /// Load shader source from file.
    bool loadSource(const FilePath& vertexShaderFile,
                    const FilePath& pixelShaderFile,
                    bool hasTransparency) override;

    /// Generate a shader from our currently stored element and
    /// the given generator context.
    bool generateShader(GenContext& context) override;

    /// Generate a shader from the given hardware shader.
    bool generateShader(ShaderPtr hwShader) override;

    /// Copy shader from one material to this one.
    void copyShader(MaterialPtr material) override;

    /// Compile the shader on first use. Throws ExceptionRenderError if
    /// compilation fails, and returns false if there is no shader.
    bool bindShader() const override;

    /// Bind viewing information for this material.
    void bindViewInformation(CameraPtr camera) override;

    /// Bind all images for this material.
    void bindImages(ImageHandlerPtr imageHandler,
                    const FileSearchPath& searchPath,
                    bool enableMipmaps = true) override;

    /// Forget the images bound by the last bindImages() call.
    void unbindImages(ImageHandlerPtr imageHandler) override;

    /// Bind a single image.
    ImagePtr bindImage(const FilePath& filePath,
                       const std::string& uniformName,
                       ImageHandlerPtr imageHandler,
                       const ImageSamplingProperties& samplingProperties) override;

    /// Bind lights, environment maps, the albedo table and shadow state.
    void bindLighting(LightHandlerPtr lightHandler,
                      ImageHandlerPtr imageHandler,
                      const ShadowState& shadowState) override;

    /// Bind the given mesh to this material.
    void bindMesh(MeshPtr mesh) override;

    /// Bind a mesh partition to this material.
    bool bindPartition(MeshPartitionPtr part) const override;

    /// Draw the given mesh partition of the bound mesh.
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

    /// Write a value to the uniform with the given shader variable name, on
    /// whichever stage declares it. Used for values outside the public
    /// uniforms, such as u_alphaThreshold. Returns false if no stage
    /// declares the uniform.
    bool bindUniform(const std::string& name, ConstValuePtr value);

    /// Write the u_time and u_frame uniforms.
    void bindTimeAndFrame(float time, float frame);

    /// Upload `image` through `textureHandler` and bind it to the named
    /// texture uniform. Returns false if the uniform does not exist.
    bool bindTexture(const std::string& uniformName, ImagePtr image,
                     HlslD3D12TextureHandlerPtr textureHandler,
                     const ImageSamplingProperties& samplingProperties);

    /// Compiled program and binding state; null before bindShader().
    HlslProgramPtr getProgram() const { return _program; }
    HlslD3D12MaterialPtr getMaterial() const { return _material; }

  protected:
    void clearShader() override;

  private:
    HlslD3D12ContextPtr _context;
    HlslD3D12GeometryCachePtr _geometryCache;

    // Shader sources loaded from files, used instead of the generated
    // shader when set.
    std::string _vertexSource;
    std::string _pixelSource;

    // Built lazily by bindShader(), which the ShaderMaterial interface
    // declares const.
    mutable HlslProgramPtr _program;
    mutable HlslD3D12MaterialPtr _material;
};

MATERIALX_NAMESPACE_END

#endif
