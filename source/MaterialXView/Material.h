#ifndef MATERIALXVIEW_MATERIAL_H
#define MATERIALXVIEW_MATERIAL_H

#include <MaterialXRender/GeometryHandler.h>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/File.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/UnitSystem.h>
#include <MaterialXRender/ImageHandler.h>
#include <MaterialXRender/LightHandler.h>

#include <nanogui/common.h>
#include <nanogui/glutil.h>

namespace mx = MaterialX;
namespace ng = nanogui;

using MaterialPtr = std::shared_ptr<class Material>;
using GLShaderPtr = std::shared_ptr<ng::GLShader>;

using StringPair = std::pair<std::string, std::string>;

class DocumentModifiers
{
  public:
    mx::StringMap remapElements;
    mx::StringSet skipElements;
    std::string filePrefixTerminator;
};

class LightingState
{
  public:
    mx::Matrix44 lightTransform;
    bool directLighting = true;
    bool indirectLighting = true;
    int envSamples = 16;
};

class ShadowState
{
  public:
    mx::ImagePtr shadowMap;
    mx::Matrix44 shadowMatrix;
    mx::ImagePtr ambientOcclusionMap;
    float ambientOcclusionGain = 0.0f;
};

class Material
{
  public:
    Material() :
        _hasTransparency(false)
    {
    }
    ~Material() { }

    static MaterialPtr create()
    {
        return std::make_shared<Material>();
    }

    /// Return the document associated with this material
    mx::DocumentPtr getDocument() const
    {
        return _doc;
    }

    /// Set the renderable element associated with this material
    void setDocument(mx::DocumentPtr doc)
    {
        _doc = doc;
    }

    /// Return the renderable element associated with this material
    mx::TypedElementPtr getElement() const
    {
        return _elem;
    }

    /// Set the renderable element associated with this material
    void setElement(mx::TypedElementPtr val)
    {
        _elem = val;
    }

    /// Return the material element associated with this material
    mx::TypedElementPtr getMaterialElement() const
    {
        return _material;
    }

    /// Set the material element associated with this material
    void setMaterialElement(mx::TypedElementPtr val)
    {
        _material = val;
    }

    /// Get any associated udim identifier
    const std::string& getUdim()
    {
        return _udim;
    }

    /// Set udim identifier
    void setUdim(const std::string& val)
    {
        _udim = val;
    }

    /// Load shader source from file.
    bool loadSource(const mx::FilePath& vertexShaderFile,
                    const mx::FilePath& pixelShaderFile,
                    const std::string& shaderName,
                    bool hasTransparency);

    /// Generate a shader from our currently stored element and
    /// the given generator context.
    bool generateShader(mx::GenContext& context);

    /// Generate a shader from the given hardware shader.
    bool generateShader(mx::ShaderPtr hwShader);

    /// Generate an environment background shader
    bool generateEnvironmentShader(mx::GenContext& context,
                                   const mx::FilePath& filename,
                                   mx::DocumentPtr stdLib,
                                   const mx::FilePath& imagePath);

    /// Copy shader from one material to this one
    void copyShader(MaterialPtr material)
    {
        _hwShader = material->_hwShader;
        _glShader = material->_glShader;
    }

    /// Return the underlying OpenGL shader.
    GLShaderPtr getShader() const
    {
        return _glShader;
    }

    /// Return true if this material has transparency.
    bool hasTransparency() const
    {
        return _hasTransparency;
    }

    /// Bind shader
    void bindShader();

    /// Bind viewing information for this material.
    void bindViewInformation(const mx::Matrix44& world, const mx::Matrix44& view, const mx::Matrix44& proj);

    /// Bind all images for this material.
    void bindImages(mx::ImageHandlerPtr imageHandler, const mx::FileSearchPath& searchPath, bool enableMipmaps = true);

    /// Unbbind all images for this material.
    void unbindImages(mx::ImageHandlerPtr imageHandler);

    /// Bind a single image.
    mx::ImagePtr bindImage(const mx::FilePath& filePath, const std::string& uniformName, mx::ImageHandlerPtr imageHandler,
                           const mx::ImageSamplingProperties& samplingProperties);

    /// Bind lights to shader.
    void bindLights(const mx::GenContext& genContext, mx::LightHandlerPtr lightHandler, mx::ImageHandlerPtr imageHandler,
                    const LightingState& lightingState, const ShadowState& shadowState);

    /// Bind units.
    void bindUnits(mx::UnitConverterRegistryPtr& registry, const mx::GenContext& context);

    /// Bind the given mesh to this material.
    void bindMesh(mx::MeshPtr mesh) const;

    /// Bind a mesh partition to this material.
    bool bindPartition(mx::MeshPartitionPtr part) const;

    /// Draw the given mesh partition.
    void drawPartition(mx::MeshPartitionPtr part) const;

    /// Return the block of public uniforms for this material.
    mx::VariableBlock* getPublicUniforms() const;

    /// Find a public uniform from its MaterialX path.
    mx::ShaderPort* findUniform(const std::string& path) const;

    /// Change the uniform value inside the shader and the associated element in the MaterialX document.
    void changeUniformElement(mx::ShaderPort* uniform, const std::string& value);

    /// Set the value for an integer element with a given path.
    void setUniformInt(const std::string& path, int value);

    /// Set the value for a float element with a given path.
    void setUniformFloat(const std::string& path, float value);

    /// Set the value for a vector2 element with a given path.
    void setUniformVec2(const std::string& path, const ng::Vector2f& value);

    /// Set the value for a vector3 element with a given path.
    void setUniformVec3(const std::string& path, const ng::Vector3f& value);

    /// Set the value for a vector4 element with a given path.
    void setUniformVec4(const std::string& path, const ng::Vector4f& value);

    /// Set the value for an enumerated element with a given path.
    void setUniformEnum(const std::string& path, int index, const std::string& value);

  protected:
    void clearShader();
    void bindUniform(const std::string& name, mx::ConstValuePtr value);
    void updateUniformsList();

  protected:
    GLShaderPtr _glShader;
    mx::ShaderPtr _hwShader;

    mx::DocumentPtr _doc;
    mx::TypedElementPtr _elem;
    mx::TypedElementPtr _material;

    std::string _udim;
    bool _hasTransparency;
    mx::StringSet _uniformVariable;

    mx::ImageVec _boundImages;
};

#endif // MATERIALXVIEW_MATERIAL_H
