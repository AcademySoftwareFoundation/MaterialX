//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GLSLMATERIAL_H
#define MATERIALX_GLSLMATERIAL_H

/// @file
/// GLSL material helper classes

#include <MaterialXRender/ShaderMaterial.h>
#include <MaterialXRenderGlsl/GlslProgram.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/UnitSystem.h>

MATERIALX_NAMESPACE_BEGIN

using GlslShaderMaterialStatePtr = std::shared_ptr<class GlslShaderMaterialState>;

/// @class GlslShaderMaterialState
/// Sub-class of ShaderMaterialState contain a glProgram that is created in generateShader.
class GlslShaderMaterialState : public ShaderMaterialState
{
  public:
    GlslShaderMaterialState(const ShaderMaterialDefinition& def) :
        ShaderMaterialState(def) { }

    /// Static creation function.
    static GlslShaderMaterialStatePtr create(const ShaderMaterialDefinition& def)
    {
        return std::make_shared<GlslShaderMaterialState>(def);
    }

    bool generateShader(GenContext& context) override;
    bool generateShader(ShaderPtr hwShader) override;

    void clearShader();
    void setProgram(GlslProgramPtr program, bool hasTransparency) { 
        _glProgram = program; 
        _hasTransparency = hasTransparency;
    }
    GlslProgramPtr getProgram() { return _glProgram; }
  protected:
    GlslProgramPtr _glProgram;
};

using GlslMaterialPtr = std::shared_ptr<class GlslMaterial>;

/// @class GlslMaterial
/// Helper class for GLSL generation and rendering of a material
class MX_RENDERGLSL_API GlslMaterial : public ShaderMaterial
{
  public:
    GlslMaterial() : ShaderMaterial()
    {
    }
    ~GlslMaterial() { }

    static GlslMaterialPtr create()
    {
        return std::make_shared<GlslMaterial>();
    }

    /// Load shader source from file.
    bool loadSource(const FilePath& vertexShaderFile,
                    const FilePath& pixelShaderFile,
                    bool hasTransparency) override;

    /// Return the underlying GLSL program.
    GlslProgramPtr getProgram() const
    {
        return _pState ? std::static_pointer_cast<GlslShaderMaterialState>(_pState)->getProgram() : nullptr;
    }

    /// Bind shader
    bool bindShader() const override;

    /// Bind viewing information for this material.
    void bindViewInformation(CameraPtr camera) override;

    /// Bind all images for this material.
    void bindImages(ImageHandlerPtr imageHandler,
                    const FileSearchPath& searchPath,
                    bool enableMipmaps = true) override;

    /// Unbbind all images for this material.
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
    
    /// Bind the uniform overrides for this material.
    void bindUniformOverrides();

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

  protected:
    void clearShader() override;
    virtual ShaderMaterialStatePtr createState() override {
        return GlslShaderMaterialState::create(_def);
    }

  private:
    GlslShaderMaterialStatePtr getState() const;
};

MATERIALX_NAMESPACE_END

#endif
