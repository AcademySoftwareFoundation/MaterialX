#ifndef MATERIALXVIEW_MATERIAL_H
#define MATERIALXVIEW_MATERIAL_H

#include <MaterialXRenderGlsl/GlslProgram.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/UnitSystem.h>

namespace mx = MaterialX;

using MaterialPtr = std::shared_ptr<class Material>;

class DocumentModifiers
{
  public:
    mx::StringMap remapElements;
    mx::StringSet skipElements;
    std::string filePrefixTerminator;
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

    /// Return the material node associated with this material
    mx::NodePtr getMaterialNode() const
    {
        return _materialNode;
    }

    /// Set the material node associated with this material
    void setMaterialNode(mx::NodePtr node)
    {
        _materialNode = node;
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
        _glProgram = material->_glProgram;
    }

    /// Return the underlying hardware shader.
    mx::ShaderPtr getShader() const
    {
        return _hwShader;
    }

    /// Return the underlying GLSL program.
    mx::GlslProgramPtr getProgram() const
    {
        return _glProgram;
    }

    /// Return true if this material has transparency.
    bool hasTransparency() const
    {
        return _hasTransparency;
    }

    /// Bind shader
    void bindShader();

    /// Bind viewing information for this material.
    void bindViewInformation(mx::CameraPtr camera);

    /// Bind all images for this material.
    void bindImages(mx::ImageHandlerPtr imageHandler, const mx::FileSearchPath& searchPath, bool enableMipmaps = true);

    /// Unbbind all images for this material.
    void unbindImages(mx::ImageHandlerPtr imageHandler);

    /// Bind a single image.
    mx::ImagePtr bindImage(const mx::FilePath& filePath, const std::string& uniformName, mx::ImageHandlerPtr imageHandler,
                           const mx::ImageSamplingProperties& samplingProperties);

    /// Bind lights to shader.
    void bindLighting(mx::LightHandlerPtr lightHandler, mx::ImageHandlerPtr imageHandler, const ShadowState& shadowState);

    /// Bind units.
    void bindUnits(mx::UnitConverterRegistryPtr& registry, const mx::GenContext& context);

    /// Bind the given mesh to this material.
    void bindMesh(mx::MeshPtr mesh);

    /// Bind a mesh partition to this material.
    bool bindPartition(mx::MeshPartitionPtr part) const;

    /// Draw the given mesh partition.
    void drawPartition(mx::MeshPartitionPtr part) const;

    /// Unbind all geometry from this material.
    void unbindGeometry();

    /// Return the block of public uniforms for this material.
    mx::VariableBlock* getPublicUniforms() const;

    /// Find a public uniform from its MaterialX path.
    mx::ShaderPort* findUniform(const std::string& path) const;

    /// Modify the value of the uniform with the given path.
    void modifyUniform(const std::string& path, mx::ConstValuePtr value, std::string valueString = mx::EMPTY_STRING);

  protected:
    void clearShader();
    void updateUniformsList();

  protected:
    mx::ShaderPtr _hwShader;
    mx::GlslProgramPtr _glProgram;

    mx::MeshPtr _boundMesh;

    mx::DocumentPtr _doc;
    mx::TypedElementPtr _elem;
    mx::NodePtr _materialNode;

    std::string _udim;
    bool _hasTransparency;
    mx::StringSet _uniformVariable;

    mx::ImageVec _boundImages;
};

#endif // MATERIALXVIEW_MATERIAL_H
