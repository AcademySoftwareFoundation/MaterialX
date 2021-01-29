#ifndef MATERIALXVIEW_VIEWER_H
#define MATERIALXVIEW_VIEWER_H

#include <MaterialXView/Camera.h>
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
           const std::string& envRadianceFilename,
           const mx::FileSearchPath& searchPath,
           const mx::FilePathVec& libraryFolders,
           int screenWidth,
           int screenHeight,
           const mx::Color3& screenColor,
           int multiSampleCount);
    ~Viewer() { }

    // Initialize the viewer for rendering.
    void initialize();

    // Set the rotation of the current mesh as Euler angles.
    void setMeshRotation(const mx::Vector3& rotation)
    {
        _meshRotation = rotation;
    }

    // Set the scale of the current mesh.
    void setMeshScale(float scale)
    {
        _meshScale = scale;
    }

    // Set the world-space position of the camera.
    void setCameraPosition(const mx::Vector3& position)
    {
        _cameraPosition = position;
    }

    // Set the world-space target of the camera.
    void setCameraTarget(const mx::Vector3& target)
    {
        _cameraTarget = target;
    }

    // Set the view angle of the camera.
    void setCameraViewAngle(float angle)
    {
        _cameraViewAngle = angle;
    }

    // Set the zoom scale of the camera.
    void setCameraZoom(float zoom)
    {
        _cameraZoom = zoom;
    }

    // Set the method for specular environment rendering.
    void setSpecularEnvironmentMethod(mx::HwSpecularEnvironmentMethod method)
    {
        _genContext.getOptions().hwSpecularEnvironmentMethod = method;
    }

    // Set the number of environment samples.
    void setEnvSampleCount(int count)
    {
        _envSampleCount = count;
    }

    // Set the rotation of the lighting environment about the Y axis.
    void setLightRotation(float rotation)
    {
        _lightRotation = rotation;
    }

    // Set the modifiers to be applied to loaded documents.
    void setDocumentModifiers(const DocumentModifiers& modifiers)
    {
        _modifiers = modifiers;
    }

    // Return the underlying NanoGUI window.
    ng::Window* getWindow() const
    {
        return _window;
    }

    // Return the active image handler.
    mx::ImageHandlerPtr getImageHandler() const
    {
        return _imageHandler;
    }

    // Return the selected material.
    MaterialPtr getSelectedMaterial() const
    {
        if (_selectedMaterial < _materials.size())
        {
            return _materials[_selectedMaterial];
        }
        return nullptr;
    }

    // Return the selected mesh partition.
    mx::MeshPartitionPtr getSelectedGeometry() const
    {
        if (_selectedGeom < _geometryList.size())
        {
            return _geometryList[_selectedGeom];
        }
        return nullptr;
    }

    // Request a capture of the current frame, writing it to the given filename.
    void requestFrameCapture(const mx::FilePath filename)
    {
        _captureRequested = true;
        _captureFilename = filename;
    }

    // Request that the viewer be closed after the next frame is rendered.
    void requestExit()
    {
        _exitRequested = true;
    }

  private:
    void drawContents() override;
    bool keyboardEvent(int key, int scancode, int action, int modifiers) override;
    bool scrollEvent(const ng::Vector2i& p, const ng::Vector2f& rel) override;
    bool mouseMotionEvent(const ng::Vector2i& p, const ng::Vector2i& rel, int button, int modifiers) override;
    bool mouseButtonEvent(const ng::Vector2i& p, int button, bool down, int modifiers) override;
    void initContext(mx::GenContext& context);

    void loadMesh(const mx::FilePath& filename);
    void loadEnvironmentLight();
    void applyDirectLights(mx::DocumentPtr doc);
    void loadDocument(const mx::FilePath& filename, mx::DocumentPtr libraries);
    void reloadShaders();
    void loadStandardLibraries();
    void saveShaderSource(mx::GenContext& context);
    void loadShaderSource();
    void saveDotFiles();

    // Assign the given material to the given geometry, or remove any
    // existing assignment if the given material is nullptr.
    void assignMaterial(mx::MeshPartitionPtr geometry, MaterialPtr material);

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

    // Return the ambient occlusion image, if any, associated with the given material.
    mx::ImagePtr getAmbientOcclusionImage(MaterialPtr material);
    
    // Split the given radiance map into indirect and direct components,
    // returning a new indirect map and directional light document.
    void splitDirectLight(mx::ImagePtr envRadianceMap, mx::ImagePtr& indirectMap, mx::DocumentPtr& dirLightDoc);

    void updateShadowMap();
    void invalidateShadowMap();

    void renderFrame();
    mx::ImagePtr getFrameImage();
    mx::ImagePtr renderWedge();
    void renderScreenSpaceQuad(MaterialPtr material);
    void bakeTextures();

    // Update the directional albedo table.
    void updateAlbedoTable();

  private:
    ng::Window* _window;
    Camera _arcball;

    mx::FilePath _materialFilename;
    mx::FilePath _meshFilename;
    mx::FilePath _envRadianceFilename;

    mx::FileSearchPath _searchPath;
    mx::FilePathVec _libraryFolders;

    mx::Vector3 _meshTranslation;
    mx::Vector3 _meshRotation;
    float _meshScale;

    mx::Vector3 _cameraPosition;
    mx::Vector3 _cameraTarget;
    mx::Vector3 _cameraUp;
    float _cameraViewAngle;
    float _cameraNearDist;
    float _cameraFarDist;
    float _cameraZoom;

    bool _userCameraEnabled;
    mx::Vector3 _userTranslation;
    mx::Vector3 _userTranslationStart;
    bool _userTranslationActive;
    ng::Vector2i _userTranslationPixel;

    // Document management
    mx::DocumentPtr _stdLib;
    DocumentModifiers _modifiers;
    mx::StringSet _xincludeFiles;

    // Lighting information
    mx::FilePath _lightRigFilename;
    mx::DocumentPtr _lightRigDoc;
    float _lightRotation;
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
    mx::ImagePtr _shadowMap;
    unsigned int _shadowSoftness;

    // Ambient occlusion
    float _ambientOcclusionGain;

    // Geometry selections
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
    mx::MeshPtr _quadMesh;

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

    // Unit options
    mx::StringVec _distanceUnitOptions;
    mx::LinearUnitConverterPtr _distanceUnitConverter;

    // Render options
    bool _renderTransparency;
    bool _renderDoubleSided;
    bool _outlineSelection;
    int _envSampleCount;
    bool _drawEnvironment;

    // Frame capture
    bool _captureRequested;
    mx::FilePath _captureFilename;
    bool _exitRequested;

    // Wedge rendering
    bool _wedgeRequested;
    mx::FilePath _wedgeFilename;
    std::string _wedgePropertyName;
    float _wedgePropertyMin;
    float _wedgePropertyMax;
    unsigned int _wedgeImageCount;

    // Texture baking
    bool _bakeTextures;
    bool _bakeHdr;
    bool _bakeAverage;
    bool _bakeOptimize;
    int _bakeTextureRes;
    bool _bakeRequested;
    mx::FilePath _bakeFilename;
};

extern const mx::Vector3 DEFAULT_CAMERA_POSITION;
extern const float DEFAULT_CAMERA_VIEW_ANGLE;
extern const float DEFAULT_CAMERA_ZOOM;
extern const int DEFAULT_ENV_SAMPLE_COUNT;

#endif // MATERIALXVIEW_VIEWER_H
