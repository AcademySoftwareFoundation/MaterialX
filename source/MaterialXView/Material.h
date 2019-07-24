#ifndef MATERIALXVIEW_MATERIAL_H
#define MATERIALXVIEW_MATERIAL_H

#include <MaterialXRender/GeometryHandler.h>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/File.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXRender/LightHandler.h>
#include <MaterialXRenderGlsl/GLTextureHandler.h>

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

    /// Generate a shader from the given inputs.
    bool generateShader(mx::GenContext& context);

    /// Copy shader from one material to this one
    void copyShader(MaterialPtr material)
    {
        _hwShader = material->_hwShader;
        _glShader = material->_glShader;
    }

    /// Generate a constant color shader
    bool generateConstantShader(mx::GenContext& context,
                                mx::DocumentPtr stdLib,
                                const std::string& shaderName,
                                const mx::Color3& color);

    /// Generate an environment background shader
    bool generateEnvironmentShader(mx::GenContext& context,
                                   const mx::FilePath& filename,
                                   mx::DocumentPtr stdLib,
                                   const mx::FilePath& imagePath);

    /// Generate an ambient occlusion shader
    bool generateAmbOccShader(mx::GenContext& context,
                              const mx::FilePath& filename,
                              mx::DocumentPtr stdLib,
                              const mx::FilePath& imagePath);

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
    void bindImages(mx::GLTextureHandlerPtr imageHandler,
                    const mx::FileSearchPath& searchPath,
                    const std::string& udim);

    /// Unbbind all images for this material.
    void unbindImages(mx::GLTextureHandlerPtr imageHandler);

    /// Bind a single image.
    mx::FilePath bindImage(const mx::FilePath& filename, const std::string& uniformName, mx::GLTextureHandlerPtr imageHandler,
                           mx::ImageDesc& desc, const mx::ImageSamplingProperties& samplingProperties, const std::string& udim = mx::EMPTY_STRING, mx::Color4* fallbackColor = nullptr);

    /// Bind lights to shader.
    void bindLights(mx::LightHandlerPtr lightHandler, mx::GLTextureHandlerPtr imageHandler, const mx::FileSearchPath& imagePath, 
                    bool directLighting, bool indirectLighting, mx::HwSpecularEnvironmentMethod specularEnvironmentMethod, int envSamples);

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

  protected:
    void bindUniform(const std::string& name, mx::ConstValuePtr value);
    void updateUniformsList();

  protected:
    GLShaderPtr _glShader;
    mx::ShaderPtr _hwShader;

    mx::DocumentPtr _doc;
    mx::TypedElementPtr _elem;

    std::string _udim;
    bool _hasTransparency;
    mx::StringSet _uniformNames;

    std::vector<mx::FilePath> _boundImages;
};

#endif // MATERIALXVIEW_MATERIAL_H
