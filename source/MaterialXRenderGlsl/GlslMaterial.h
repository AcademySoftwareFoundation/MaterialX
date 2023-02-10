//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GLSLMATERIAL_H
#define MATERIALX_GLSLMATERIAL_H

/// @file
/// GLSL material helper classes

#include <MaterialXRenderGlsl/GlslProgram.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/UnitSystem.h>

MATERIALX_NAMESPACE_BEGIN

using GlslMaterialPtr = std::shared_ptr<class GlslMaterial>;

/// @class ShadowState
/// Helper class representing shadow rendering state
class MX_RENDERGLSL_API ShadowState
{
  public:
    ImagePtr shadowMap;
    Matrix44 shadowMatrix;
    ImagePtr ambientOcclusionMap;
    float ambientOcclusionGain = 0.0f;
};

/// @class GlslMaterial
/// Helper class for GLSL generation and rendering of a material
class MX_RENDERGLSL_API GlslMaterial
{
  public:
    GlslMaterial() :
        _hasTransparency(false)
    {
    }
    ~GlslMaterial() { }

    static GlslMaterialPtr create()
    {
        return std::make_shared<GlslMaterial>();
    }

    /// Set the renderable element associated with this material
    void setDocument(DocumentPtr doc)
    {
        _doc = doc;
    }

    /// Return the document associated with this material
    DocumentPtr getDocument() const
    {
        return _doc;
    }

    /// Set the renderable element associated with this material
    void setElement(TypedElementPtr val)
    {
        _elem = val;
    }

    /// Return the renderable element associated with this material
    TypedElementPtr getElement() const
    {
        return _elem;
    }

    /// Set the material node associated with this material
    void setMaterialNode(NodePtr node)
    {
        _materialNode = node;
    }

    /// Return the material node associated with this material
    NodePtr getMaterialNode() const
    {
        return _materialNode;
    }

    /// Set udim identifier
    void setUdim(const std::string& val)
    {
        _udim = val;
    }

    /// Get any associated udim identifier
    const std::string& getUdim()
    {
        return _udim;
    }

    /// Load shader source from file.
    bool loadSource(const FilePath& vertexShaderFile,
                    const FilePath& pixelShaderFile,
                    bool hasTransparency);

    /// Generate a shader from our currently stored element and
    /// the given generator context.
    bool generateShader(GenContext& context);

    /// Generate a shader from the given hardware shader.
    bool generateShader(ShaderPtr hwShader);

    /// Generate an environment background shader
    bool generateEnvironmentShader(GenContext& context,
                                   const FilePath& filename,
                                   DocumentPtr stdLib,
                                   const FilePath& imagePath);

    /// Copy shader from one material to this one
    void copyShader(GlslMaterialPtr material)
    {
        _hwShader = material->_hwShader;
        _glProgram = material->_glProgram;
    }

    /// Return the underlying hardware shader.
    ShaderPtr getShader() const
    {
        return _hwShader;
    }

    /// Return the underlying GLSL program.
    GlslProgramPtr getProgram() const
    {
        return _glProgram;
    }

    /// Return true if this material has transparency.
    bool hasTransparency() const
    {
        return _hasTransparency;
    }

    /// Bind shader
    bool bindShader() const;

    /// Bind viewing information for this material.
    void bindViewInformation(CameraPtr camera);

    /// Bind all images for this material.
    void bindImages(ImageHandlerPtr imageHandler, const FileSearchPath& searchPath, bool enableMipmaps = true);

    /// Unbbind all images for this material.
    void unbindImages(ImageHandlerPtr imageHandler);

    /// Bind a single image.
    ImagePtr bindImage(const FilePath& filePath, const std::string& uniformName, ImageHandlerPtr imageHandler,
                           const ImageSamplingProperties& samplingProperties);

    /// Bind lights to shader.
    void bindLighting(LightHandlerPtr lightHandler, ImageHandlerPtr imageHandler, const ShadowState& shadowState);

    /// Bind units.
    void bindUnits(UnitConverterRegistryPtr& registry, const GenContext& context);

    /// Bind the given mesh to this material.
    void bindMesh(MeshPtr mesh);

    /// Bind a mesh partition to this material.
    bool bindPartition(MeshPartitionPtr part) const;

    /// Draw the given mesh partition.
    void drawPartition(MeshPartitionPtr part) const;

    /// Unbind all geometry from this material.
    void unbindGeometry();

    /// Return the block of public uniforms for this material.
    VariableBlock* getPublicUniforms() const;

    /// Find a public uniform from its MaterialX path.
    ShaderPort* findUniform(const std::string& path) const;

    /// Modify the value of the uniform with the given path.
    void modifyUniform(const std::string& path, ConstValuePtr value, std::string valueString = EMPTY_STRING);

  protected:
    void clearShader();

  protected:
    ShaderPtr _hwShader;
    GlslProgramPtr _glProgram;

    MeshPtr _boundMesh;

    DocumentPtr _doc;
    TypedElementPtr _elem;
    NodePtr _materialNode;

    std::string _udim;
    bool _hasTransparency;

    ImageVec _boundImages;
};

MATERIALX_NAMESPACE_END

#endif
