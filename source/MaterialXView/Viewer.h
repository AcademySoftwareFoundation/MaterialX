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

    mx::DocumentPtr getCurrentDocument() const
    {
        return _doc;
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
    void setupLights(mx::DocumentPtr doc, const std::string& envRadiancePath, const std::string& envIrradiancePath);
    void initializeDocument(mx::DocumentPtr libraries);
    void reloadDocument();
    void saveShaderSource();
    void loadShaderSource();
    void saveDotFiles();

    /// Assign the given material to a geometry, or to all geometries if no
    /// target is specified.
    void assignMaterial(MaterialPtr material, mx::MeshPartitionPtr geometry = nullptr);
    void initCamera();
    void computeCameraMatrices(mx::Matrix44& world,
                               mx::Matrix44& view,
                               mx::Matrix44& proj);

    bool setGeometrySelection(size_t index);
    void updateGeometrySelections();

    MaterialPtr setMaterialSelection(size_t index);
    void updateMaterialSelections();

    void updatePropertyEditor();

    void createLoadMeshInterface(Widget* parent, const std::string label);
    void createLoadMaterialsInterface(Widget* parent, const std::string label);
    void createAdvancedSettings(Widget* parent);

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
    mx::DocumentPtr _doc;
    mx::FilePath _materialFilename;
    DocumentModifiers _modifiers;

    // Lighting information
    std::string _lightFileName;
    std::string _envRadiancePath;
    std::string _envIrradiancePath;
    bool _directLighting;
    bool _indirectLighting;

    // Geometry selections
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
};

#endif // MATERIALXVIEW_VIEWER_H
