#ifndef MATERIALXVIEW_MATERIAL_H
#define MATERIALXVIEW_MATERIAL_H

#include <MaterialXRender/Handlers/GeometryHandler.h>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/File.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXRender/Handlers/HwLightHandler.h>
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
    Material() {}
    ~Material() { }

    static MaterialPtr create()
    {
        return std::make_shared<Material>();
    }

    /// Load a document on disk containing renderable materials into an existing document
    /// and create new materials if they do not already exist.
    /// Returns the number of new materials added
    static size_t loadDocument(mx::DocumentPtr destinationDoc, const mx::FilePath& filePath,
                               mx::DocumentPtr libraries, const DocumentModifiers& modifiers,
                               std::vector<MaterialPtr>& materials);

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
    bool loadSource(const mx::FilePath& vertexStage, const mx::FilePath& pixelStage, const std::string& shaderName, bool hasTransparency);

    /// Generate a shader from the given inputs.
    bool generateShader(mx::GenContext& context);

    /// Generate a constant color shader
    bool generateConstantShader(mx::GenContext& context,
                                mx::DocumentPtr stdLib,
                                const std::string& shaderName,
                                const mx::Color3& color);

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
    void bindShader(mx::GenContext& context);

    /// Bind viewing information for this material.
    void bindViewInformation(const mx::Matrix44& world, const mx::Matrix44& view, const mx::Matrix44& proj);

    /// Bind all images for this material.
    void bindImages(mx::GLTextureHandlerPtr imageHandler, const mx::FileSearchPath& imagePath, const std::string& udim);

    /// Bind a single image.
    bool bindImage(std::string filename, const std::string& uniformName, mx::GLTextureHandlerPtr imageHandler,
                   mx::ImageDesc& desc, const std::string& udim = mx::EMPTY_STRING, mx::Color4* fallbackColor = nullptr);

    /// Bind lights to shader.
    void bindLights(mx::HwLightHandlerPtr lightHandler, mx::GLTextureHandlerPtr imageHandler, const mx::FileSearchPath& imagePath, 
                    int envSamples, bool directLighting, bool indirectLighting);

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

    GLShaderPtr _glShader;
    mx::ShaderPtr _hwShader;
    mx::TypedElementPtr _elem;
    std::string _udim;
    bool _hasTransparency;
    mx::StringSet _uniformNames;
};

#endif // MATERIALXVIEW_MATERIAL_H
