//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_GLSLRENDERVIEW_H
#define MATERIALX_GLSLRENDERVIEW_H

#include <MaterialXGraphEditor/RenderViewBase.h>

#include <MaterialXRenderGlsl/GLFramebuffer.h>
#include <MaterialXRenderGlsl/GlslMaterial.h>

#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/LightHandler.h>
#include <MaterialXRender/Timer.h>

namespace mx = MaterialX;

class GlslRenderView;
using GlslRenderViewPtr = std::shared_ptr<GlslRenderView>;

class DocumentModifiers
{
  public:
    mx::StringMap remapElements;
    mx::StringSet skipElements;
    std::string filePrefixTerminator;
};

class GlslRenderView : public RenderViewBase
{
  public:
    GlslRenderView(mx::DocumentPtr doc,
                   mx::DocumentPtr stdLib,
                   const std::string& meshFilename,
                   const std::string& envRadianceFilename,
                   const mx::FileSearchPath& searchPath,
                   int viewWidth,
                   int viewHeight);
    ~GlslRenderView() = default;

    // Return the name of this render backend.
    std::string getBackendName() const override
    {
        return "GLSL";
    }

    // Initialize the viewer for rendering.
    void initialize() override;

    // Set the method for specular environment rendering.
    void setSpecularEnvironmentMethod(mx::HwSpecularEnvironmentMethod method)
    {
        _genContext.getOptions().hwSpecularEnvironmentMethod = method;
    }

    // Set the number of environment samples.
    void setEnvSampleCount(int count)
    {
        _lightHandler->setEnvSampleCount(count);
    }

    // Set the rotation of the lighting environment about the Y axis.
    void setLightRotation(float rotation)
    {
        _lightRotation = rotation;
    }

    // Enable or disable shadow maps.
    void setShadowMapEnable(bool enable)
    {
        _genContext.getOptions().hwShadowMap = enable;
    }

    // Set the modifiers to be applied to loaded documents.
    void setDocumentModifiers(const DocumentModifiers& modifiers)
    {
        _modifiers = modifiers;
    }

    std::vector<mx::MeshPartitionPtr> getGeometryList()
    {
        return _geometryList;
    }

    mx::FileSearchPath getMaterialSearchPath()
    {
        return _materialSearchPath;
    }

    // Return the pixel ratio.
    float getPixelRatio() const override
    {
        return _pixelRatio;
    }

    // Return the active image handler.
    mx::ImageHandlerPtr getImageHandler() const override
    {
        return _imageHandler;
    }

    // Return the selected material.
    mx::GlslMaterialPtr getSelectedMaterial() const
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

    mx::GenContext& getGenContext()
    {
        return _genContext;
    }

    std::map<mx::MeshPartitionPtr, mx::GlslMaterialPtr> getMaterialAssignments()
    {
        return _materialAssignments;
    }

    std::vector<mx::GlslMaterialPtr> getMaterials()
    {
        return _materials;
    }

    mx::CameraPtr getViewCamera() override
    {
        return _viewCamera;
    }

    const mx::StringSet& getXincludeFiles() const override
    {
        return _xincludeFiles;
    }

    // Request a capture of the current frame, writing it to the given filename.
    void requestFrameCapture(const mx::FilePath& filename) override
    {
        _captureRequested = true;
        _captureFilename = filename;
    }

    // Request that the viewer be closed after the next frame is rendered.
    void requestExit() override
    {
        _exitRequested = true;
    }

    float getCameraZoom()
    {
        return _cameraZoom;
    }

    void setCameraZoom(float amount)
    {
        _cameraZoom = amount;
    }

    // Return the OpenGL texture ID of the most recently rendered frame.
    unsigned int getRenderTextureId() const override
    {
        return _textureID;
    }

    // Called before the rendered frame is drawn by ImGui.
    void beginFrameDisplay() override;

    // Called after the rendered frame is drawn by ImGui.
    void endFrameDisplay() override;

    // Return true if the given node definition is supported by the GLSL backend.
    bool isNodeDefSupported(const mx::NodeDefPtr& nodeDef) override;

    // Update a uniform in the currently selected material.
    void modifyUniform(const std::string& name, mx::ValuePtr value) override;

    void drawContents() override;
    void reloadShaders();

    void setDocument(mx::DocumentPtr document) override;
    void assignMaterial(mx::MeshPartitionPtr geometry, mx::GlslMaterialPtr material);
    void updateMaterials(mx::TypedElementPtr typedElem) override;
    void setMouseButtonEvent(int button, bool down, mx::Vector2 pos) override;
    void setMouseMotionEvent(mx::Vector2 pos) override;
    void setKeyEvent(int key) override;
    void setScrollEvent(float scrollY) override;
    void setMaterial(mx::TypedElementPtr elem);

    void loadMesh(const mx::FilePath& filename) override;

  private:
    void initContext(mx::GenContext& context);
    void loadEnvironmentLight();
    void applyDirectLights(mx::DocumentPtr doc);

    // Mark the given material as currently selected in the view.
    void setSelectedMaterial(mx::GlslMaterialPtr material)
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
    void updateCameras();
    void updateGeometrySelections();

    mx::ImagePtr getShadowMap();

    void renderFrame();
    void renderScreenSpaceQuad(mx::GlslMaterialPtr material);

  private:
    mx::FileSearchPath _materialSearchPath;
    mx::FilePath _meshFilename;
    mx::FilePath _envRadianceFilename;

    mx::FileSearchPath _searchPath;

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

    float _pixelRatio;
    mx::GLFramebufferPtr _renderFrame;
    unsigned int _textureID;

    mx::Vector3 _userTranslation;
    mx::Vector3 _userTranslationStart;
    bool _userTranslationActive;
    mx::Vector2 _userTranslationPixel;

    // Document management
    mx::DocumentPtr _document;
    mx::DocumentPtr _stdLib;
    DocumentModifiers _modifiers;
    mx::StringSet _xincludeFiles;

    // Lighting information
    mx::FilePath _lightRigFilename;
    mx::DocumentPtr _lightRigDoc;
    float _lightRotation;

    // Shadow mapping
    mx::GlslMaterialPtr _shadowMaterial;
    mx::GlslMaterialPtr _shadowBlurMaterial;
    mx::ImagePtr _shadowMap;
    mx::ImagePtr _graphRender;
    unsigned int _shadowSoftness;

    // Geometry selections
    std::vector<mx::MeshPartitionPtr> _geometryList;
    size_t _selectedGeom;

    // Material selections
    std::vector<mx::GlslMaterialPtr> _materials;
    mx::GlslMaterialPtr _wireMaterial;
    size_t _selectedMaterial;

    // Material assignments
    std::map<mx::MeshPartitionPtr, mx::GlslMaterialPtr> _materialAssignments;

    // Cameras
    mx::CameraPtr _viewCamera;
    mx::CameraPtr _envCamera;
    mx::CameraPtr _shadowCamera;

    // Resource handlers
    mx::GeometryHandlerPtr _geometryHandler;
    mx::ImageHandlerPtr _imageHandler;
    mx::LightHandlerPtr _lightHandler;

    // Supporting geometry.
    mx::MeshPtr _quadMesh;

    // Shader generator context
    mx::GenContext _genContext;

    // Unit registry
    mx::UnitConverterRegistryPtr _unitRegistry;

    // Mesh options
    bool _splitByUdims;

    // Unit options
    mx::StringVec _distanceUnitOptions;
    mx::LinearUnitConverterPtr _distanceUnitConverter;

    // Render options
    bool _renderTransparency;
    bool _renderDoubleSided;

    // Frame capture
    bool _captureRequested;
    mx::FilePath _captureFilename;
    bool _exitRequested;

    // Time and frame
    mx::ScopedTimer _timer;
};

extern const mx::Vector3 DEFAULT_CAMERA_POSITION;
extern const float DEFAULT_CAMERA_VIEW_ANGLE;
extern const float DEFAULT_CAMERA_ZOOM;

#endif
