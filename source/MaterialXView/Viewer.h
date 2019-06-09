#ifndef MATERIALXVIEW_VIEWER_H
#define MATERIALXVIEW_VIEWER_H

#include <MaterialXView/Editor.h>
#include <MaterialXView/Material.h>
#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/LightHandler.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>

namespace mx = MaterialX;
namespace ng = nanogui;

class Viewer : public ng::Screen
{
  public:
    Viewer(const mx::StringVec& libraryFolders,
           const mx::FileSearchPath& searchPath,
           const std::string& meshFilename,
           const std::string& materialFilename,
           const DocumentModifiers& modifiers,
           mx::HwSpecularEnvironmentMethod specularEnvironmentMethod,
           const std::string& envRadiancePath,
           const std::string& envIrradiancePath,
           int multiSampleCount);
    ~Viewer() { }

    void drawContents() override;
    bool keyboardEvent(int key, int scancode, int action, int modifiers) override;

    bool scrollEvent(const ng::Vector2i& p, const ng::Vector2f& rel) override;
    bool mouseMotionEvent(const ng::Vector2i& p, const ng::Vector2i& rel, int button, int modifiers) override;
    bool mouseButtonEvent(const ng::Vector2i& p, int button, bool down, int modifiers) override;

    ng::Window* getWindow() const
    {
        return _window;
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

    const mx::GLTextureHandlerPtr getImageHandler() const
    {
        return _imageHandler;
    }

  private:
    void drawScene3D();
    void drawScene2D();

    void setupLights(mx::DocumentPtr doc);
    void loadDocument(const mx::FilePath& filename, mx::DocumentPtr libraries);
    void saveShaderSource();
    void loadShaderSource();
    void saveDotFiles();

    /// Assign the given material to the given geometry, or remove any
    /// existing assignment if the given material is nullptr.
    void assignMaterial(mx::MeshPartitionPtr geometry, MaterialPtr material);
    void initCamera();
    void computeCameraMatrices(mx::Matrix44& world,
                               mx::Matrix44& view,
                               mx::Matrix44& proj);

    void updateGeometrySelections();
    void updateMaterialSelections();
    void updateMaterialSelectionUI();
    void updatePropertyEditor();

    void createLoadMeshInterface(Widget* parent, const std::string& label);
    void createLoadMaterialsInterface(Widget* parent, const std::string& label);
    void createAdvancedSettings(Widget* parent);

    mx::MeshStreamPtr createUvPositionStream(mx::MeshPtr mesh, 
                                            const std::string& uvStreamName,
                                            unsigned int index,
                                            const std::string& positionStreamName);

  private:
    ng::Window* _window;
    ng::Arcball _arcball;

    mx::Vector3 _eye;
    mx::Vector3 _center;
    mx::Vector3 _up;
    float _zoom;
    float _viewAngle;
    float _nearDist;
    float _farDist;

    mx::Vector3 _modelTranslation;
    mx::Vector3 _modelTranslationStart;
    float _modelZoom;

    bool _translationActive;
    ng::Vector2i _translationStart;

    // Document management
    mx::StringVec _libraryFolders;
    mx::FileSearchPath _searchPath;
    mx::DocumentPtr _stdLib;
    mx::FilePath _materialFilename;
    DocumentModifiers _modifiers;

    // Lighting information
    std::string _lightFileName;
    std::string _envRadiancePath;
    std::string _envIrradiancePath;
    bool _directLighting;
    bool _indirectLighting;
    bool _ambientOcclusion;

    // Geometry selections
    std::string _meshFilename;
    std::vector<mx::MeshPartitionPtr> _geometryList;
    size_t _selectedGeom;
    ng::Label* _geomLabel;
    ng::ComboBox* _geometryListBox;

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
    mx::GLTextureHandlerPtr _imageHandler;
    mx::LightHandlerPtr _lightHandler;

    // Supporting materials and geometry.
    MaterialPtr _ambOccMaterial;
    mx::GeometryHandlerPtr _envGeometryHandler;
    MaterialPtr _envMaterial;

    // Shader generator
    mx::GenContext _genContext;

    // Mesh options
    bool _splitByUdims;

    // Material options
    bool _mergeMaterials;

    // Render options
    bool _outlineSelection;
    mx::HwSpecularEnvironmentMethod _specularEnvironmentMethod;
    int _envSamples;
    bool _drawEnvironment;
    mx::Matrix44 _envMatrix;

    // Image save
    bool _captureFrame;
    mx::FilePath _captureFrameFileName;

    // UV wireframe drawing
    bool _drawUVGeometry;
    MaterialPtr _wireMaterialUV;
    mx::Vector3 _uvScale;
    mx::Vector3 _uvTranslation;
    float _uvZoom;
};

#endif // MATERIALXVIEW_VIEWER_H
