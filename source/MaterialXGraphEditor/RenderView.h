//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_RENDERVIEW_H
#define MATERIALX_RENDERVIEW_H

#include <MaterialXRenderGlsl/GLFramebuffer.h>
#include <MaterialXRenderGlsl/GlslMaterial.h>

#include <MaterialXRender/Camera.h>
#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/LightHandler.h>
#include <MaterialXRender/Timer.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXCore/Unit.h>

#include "imgui_impl_glfw.h"

namespace mx = MaterialX;

class DocumentModifiers
{
  public:
    mx::StringMap remapElements;
    mx::StringSet skipElements;
    std::string filePrefixTerminator;
};

class RenderView
{
  public:
    RenderView(mx::DocumentPtr doc,
               const std::string& meshFilename,
               const std::string& envRadianceFilename,
               const mx::FileSearchPath& searchPath,
               unsigned int screenWidth,
               unsigned int screenHeight);
    ~RenderView() { }

    // Initialize the viewer for rendering.
    void initialize();

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

    // Return the active image handler.
    mx::ImageHandlerPtr getImageHandler() const
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

    bool getMergeMaterials()
    {
        return _mergeMaterials;
    }

    std::vector<mx::GlslMaterialPtr> getMaterials()
    {
        return _materials;
    }

    mx::CameraPtr getViewCamera()
    {
        return _viewCamera;
    }

    const mx::StringSet& getXincludeFiles() const
    {
        return _xincludeFiles;
    }
    mx::ElementPredicate getElementPredicate();

    // Request a capture of the current frame, writing it to the given filename.
    void requestFrameCapture(const mx::FilePath& filename)
    {
        _captureRequested = true;
        _captureFilename = filename;
    }

    // Request that the viewer be closed after the next frame is rendered.
    void requestExit()
    {
        _exitRequested = true;
    }

    // return user camera enabled
    bool getUserCameraEnabled()
    {
        return _userCameraEnabled;
    }

    float getCameraZoom()
    {
        return _cameraZoom;
    }

    void setCameraZoom(float amount)
    {
        _cameraZoom = amount;
    }

    bool getMaterialCompilation()
    {
        return _materialCompilation;
    }

    void setMaterialCompilation(bool mat)
    {
        _materialCompilation = mat;
    }

    void drawContents();
    unsigned int _textureID;
    void reloadShaders();

    float _pixelRatio;
    unsigned int _screenWidth;
    unsigned int _screenHeight;
    mx::GLFramebufferPtr _renderFrame;
    void setDocument(mx::DocumentPtr document);
    void assignMaterial(mx::MeshPartitionPtr geometry, mx::GlslMaterialPtr material);
    void updateMaterials(mx::TypedElementPtr typedElem);
    void setMouseButtonEvent(int button, bool down, mx::Vector2 pos);
    void setMouseMotionEvent(mx::Vector2 pos);
    void setKeyEvent(int key);
    void setScrollEvent(float scrollY);
    void setMaterial(mx::TypedElementPtr elem);

  private:
    void initContext(mx::GenContext& context);
    void loadMesh(const mx::FilePath& filename);
    void loadEnvironmentLight();
    void applyDirectLights(mx::DocumentPtr doc);

    // Mark the given material as currently selected in the viewer.
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

    // Return an element predicate for documents written from the viewer.

    void initCamera();
    void updateCameras();
    void updateGeometrySelections();

    // Return the ambient occlusion image, if any, associated with the given material.
    mx::ImagePtr getAmbientOcclusionImage(mx::GlslMaterialPtr material);
    mx::GlslMaterialPtr getWireframeMaterial();

    mx::ImagePtr getShadowMap();
    mx::ImagePtr _renderMap;

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

    bool _userCameraEnabled;
    mx::Vector3 _userTranslation;
    mx::Vector3 _userTranslationStart;
    bool _userTranslationActive;
    mx::Vector2 _userTranslationPixel;

    // Document management
    mx::DocumentPtr _document;
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

    // Ambient occlusion
    float _ambientOcclusionGain;

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

    // Material options
    bool _mergeMaterials;
    bool _materialCompilation;

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
};

extern const mx::Vector3 DEFAULT_CAMERA_POSITION;
extern const float DEFAULT_CAMERA_VIEW_ANGLE;
extern const float DEFAULT_CAMERA_ZOOM;

#endif
