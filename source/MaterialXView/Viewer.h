#ifndef MATERIALXVIEW_VIEWER_H
#define MATERIALXVIEW_VIEWER_H

#include <MaterialXView/Editor.h>
#include <MaterialXView/Material.h>

#include <MaterialXRenderGlsl/GLFramebuffer.h>

#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/LightHandler.h>
#include <MaterialXRender/ViewHandler.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>

#include <MaterialXCore/Unit.h>

namespace mx = MaterialX;
namespace ng = nanogui;

class Viewer : public ng::Screen
{
  public:
    Viewer(const std::string& materialFilename,
           const std::string& meshFilename,
           const mx::FilePathVec& libraryFolders,
           const mx::FileSearchPath& searchPath,
           const DocumentModifiers& modifiers,
           mx::HwSpecularEnvironmentMethod specularEnvironmentMethod,
           const std::string& envRadiancePath,
           int multiSampleCount);
    ~Viewer() { }

    void drawContents() override;
    bool keyboardEvent(int key, int scancode, int action, int modifiers) override;

    bool scrollEvent(const ng::Vector2i& p, const ng::Vector2f& rel) override;
    bool mouseMotionEvent(const ng::Vector2i& p, const ng::Vector2i& rel, int button, int modifiers) override;
    bool mouseButtonEvent(const ng::Vector2i& p, int button, bool down, int modifiers) override;

    void renderFrame();
    mx::ImagePtr getFrameImage();
    mx::ImagePtr renderWedge();
    void bakeTextures();

    ng::Window* getWindow() const
    {
        return _window;
    }

    void setSelectedMaterial(MaterialPtr material)
    {
        for (size_t i = 0; i < _materials.size(); i++)
        {
            if (material == _materials[i])
            {
                _selectedMaterial = i;
                break;
            }
        }
    }

    MaterialPtr getSelectedMaterial() const
    {
        if (_selectedMaterial < _materials.size())
        {
            return _materials[_selectedMaterial];
        }
        return nullptr;
    }

    mx::MeshPartitionPtr getSelectedGeometry() const
    {
        if (_selectedGeom < _geometryList.size())
        {
            return _geometryList[_selectedGeom];
        }
        return nullptr;
    }

    const mx::FileSearchPath& getSearchPath() const
    {
        return _searchPath;
    }

    mx::ImageHandlerPtr getImageHandler() const
    {
        return _imageHandler;
    }

    bool showAdvancedProperties() const
    {
        return _showAdvancedProperties;
    }

  private:
    void initContext(mx::GenContext& context);
    void loadEnvironmentLight();
    void applyDirectLights(mx::DocumentPtr doc);
    void loadDocument(const mx::FilePath& filename, mx::DocumentPtr libraries);
    void reloadShaders();
    void loadStandardLibraries();
    void saveShaderSource(mx::GenContext& context);
    void loadShaderSource();
    void saveDotFiles();

    /// Assign the given material to the given geometry, or remove any
    /// existing assignment if the given material is nullptr.
    void assignMaterial(mx::MeshPartitionPtr geometry, MaterialPtr material);

    void initCamera();
    void updateViewHandlers();
    void updateGeometrySelections();
    void updateMaterialSelections();
    void updateMaterialSelectionUI();
    void updateDisplayedProperties();

    void createLoadMeshInterface(Widget* parent, const std::string& label);
    void createLoadMaterialsInterface(Widget* parent, const std::string& label);
    void createLoadEnvironmentInterface(Widget* parent, const std::string& label);
    void createSaveMaterialsInterface(Widget* parent, const std::string& label);
    void createPropertyEditorInterface(Widget* parent, const std::string& label);
    void createAdvancedSettings(Widget* parent);

    /// Return the ambient occlusion image, if any, associated with the given material.
    mx::ImagePtr getAmbientOcclusionImage(MaterialPtr material);
    
    /// Split the given radiance map into indirect and direct components,
    /// returning a new indirect map and directional light document.
    void splitDirectLight(mx::ImagePtr envRadianceMap, mx::ImagePtr& indirectMap, mx::DocumentPtr& dirLightDoc);

    /// Update the current shadow map.
    void updateShadowMap();

    /// Check for any OpenGL errors that have been encountered.
    void checkGlErrors(const std::string& context);

  private:
    ng::Window* _window;
    ng::Arcball _arcball;

    mx::Vector3 _eye;
    mx::Vector3 _center;
    mx::Vector3 _up;
    float _viewAngle;
    float _nearDist;
    float _farDist;
    float _cameraYaw;

    float _modelZoom;
    mx::Vector3 _modelTranslation;
    float _modelYaw;

    float _userZoom;
    mx::Vector3 _userTranslation;
    mx::Vector3 _userTranslationStart;
    bool _userTranslationActive;
    ng::Vector2i _userTranslationPixel;

    // Document management
    mx::FilePathVec _libraryFolders;
    mx::FileSearchPath _searchPath;
    mx::DocumentPtr _stdLib;
    mx::FilePath _materialFilename;
    DocumentModifiers _modifiers;
    mx::StringSet _xincludeFiles;

    // Lighting information
    mx::FilePath _envRadiancePath;
    mx::FilePath _lightRigFilename;
    mx::DocumentPtr _lightRigDoc;
    bool _directLighting;
    bool _indirectLighting;

    // Light processing options
    bool _normalizeEnvironment;
    bool _splitDirectLight;
    bool _generateReferenceIrradiance;
    bool _saveGeneratedLights;

    // Shadow mapping
    MaterialPtr _shadowMaterial;
    MaterialPtr _shadowBlurMaterial;
    mx::GLFrameBufferPtr _shadowFramebuffer;
    mx::ImagePtr _shadowMap;
    unsigned int _shadowSoftness;

    // Ambient occlusion
    float _ambientOcclusionGain;

    // Geometry selections
    mx::FilePath _meshFilename;
    std::vector<mx::MeshPartitionPtr> _geometryList;
    size_t _selectedGeom;
    ng::Label* _geomLabel;
    ng::ComboBox* _geometrySelectionBox;

    // Material selections
    std::vector<MaterialPtr> _materials;
    MaterialPtr _wireMaterial;
    size_t _selectedMaterial;
    ng::Label* _materialLabel;
    ng::ComboBox* _materialSelectionBox;
    PropertyEditor _propertyEditor;

    // Material assignments
    std::map<mx::MeshPartitionPtr, MaterialPtr> _materialAssignments;

    // Resource handlers
    mx::GeometryHandlerPtr _geometryHandler;
    mx::ImageHandlerPtr _imageHandler;
    mx::LightHandlerPtr _lightHandler;

    // View handlers
    mx::ViewHandlerPtr _cameraViewHandler;
    mx::ViewHandlerPtr _shadowViewHandler;

    // Supporting materials and geometry.
    mx::GeometryHandlerPtr _envGeometryHandler;
    MaterialPtr _envMaterial;

    // Shader generator contexts
    mx::GenContext _genContext;
#if MATERIALX_BUILD_GEN_OSL
    mx::GenContext _genContextOsl;
#endif
#if MATERIALX_BUILD_GEN_MDL
    mx::GenContext _genContextMdl;
#endif

    // Unit registry
    mx::UnitConverterRegistryPtr _unitRegistry;

    // Mesh options
    bool _splitByUdims;

    // Material options
    bool _mergeMaterials;
    bool _bakeTextures;

    // Unit options
    mx::StringVec _distanceUnitOptions;
    ng::ComboBox* _distanceUnitBox;
    mx::LinearUnitConverterPtr _distanceUnitConverter;

    // Render options
    bool _outlineSelection;
    mx::HwSpecularEnvironmentMethod _specularEnvironmentMethod;
    int _envSamples;
    bool _drawEnvironment;
    mx::Matrix44 _envMatrix;

    // Property options
    bool _showAdvancedProperties;

    // Frame capture
    bool _captureRequested;
    mx::FilePath _captureFilename;

    // Wedge rendering
    bool _wedgeRequested;
    mx::FilePath _wedgeFilename;
    std::string _wedgePropertyName;
    float _wedgePropertyMin;
    float _wedgePropertyMax;
    unsigned int _wedgeImageCount;

    // Texture baking
    bool _bakeRequested;
    mx::FilePath _bakeFilename;
};

#endif // MATERIALXVIEW_VIEWER_H
