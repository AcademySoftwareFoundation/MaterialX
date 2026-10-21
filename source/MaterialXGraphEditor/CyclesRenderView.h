//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_CYCLESRENDERVIEW_H
#define MATERIALX_CYCLESRENDERVIEW_H

#ifdef MATERIALX_BUILD_RENDER_CYCLES

#include <MaterialXGraphEditor/RenderViewBase.h>

#include <MaterialXRender/GeometryHandler.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenOsl/OslShaderGenerator.h>

#include "session/display_driver.h"

#include <memory>
#include <vector>

namespace mx = MaterialX;

class CyclesRenderView;
using CyclesRenderViewPtr = std::shared_ptr<CyclesRenderView>;

class CyclesCaptureDisplayDriver;

namespace ccl {
class Session;
class Scene;
class Object;
class Light;
class BackgroundNode;
class SessionParams;
class SceneParams;
} // namespace ccl

/// @class CyclesRenderView
/// Cycles render view embedded in the graph editor.
///
/// Renders an interactive path-traced preview using the Cycles renderer,
/// capturing the rendered frame and presenting it to the graph editor as an
/// OpenGL texture.
class CyclesRenderView : public RenderViewBase
{
  public:
    CyclesRenderView(mx::DocumentPtr doc,
                     mx::DocumentPtr stdLib,
                     const std::string& meshFilename,
                     const std::string& envRadianceFilename,
                     const mx::FileSearchPath& searchPath,
                     int viewWidth,
                     int viewHeight,
                     const std::string& renderPass = "combined");
    ~CyclesRenderView() override;

    // Return the name of this render backend.
    std::string getBackendName() const override
    {
        return "Cycles";
    }

    // Initialize the viewer for rendering.
    void initialize() override;

    void setDocument(mx::DocumentPtr document) override;
    void updateMaterials(mx::TypedElementPtr typedElem) override;
    void loadMesh(const mx::FilePath& filename) override;
    void drawContents() override;

    // Return the OpenGL texture ID of the most recently rendered frame.
    unsigned int getRenderTextureId() const override
    {
        return _textureID;
    }

    // Return the camera used to view the rendered scene.
    mx::CameraPtr getViewCamera() override
    {
        return _viewCamera;
    }

    // Return the pixel ratio of the render view.
    float getPixelRatio() const override
    {
        return 1.0f;
    }

    // Toggle sRGB encoding while the render view is drawn by ImGui, so the
    // linear display texture is encoded like the captured images. Mirrors
    // GlslRenderView::beginFrameDisplay.
    void beginFrameDisplay() override;

    void endFrameDisplay() override;

    // Camera interaction: left-drag orbits, right/shift-left-drag pans and
    // scroll or keypad +/- zoom. Camera changes restart the progressive render.
    void setMouseButtonEvent(int button, bool down, mx::Vector2 pos) override;
    void setMouseMotionEvent(mx::Vector2 pos) override;
    void setKeyEvent(int key) override;
    void setScrollEvent(float scrollY) override;

    // Request a capture of the current frame, writing it to the given filename.
    void requestFrameCapture(const mx::FilePath& filename) override;

    // Return true once the requested frame capture has been written.
    bool isFrameCaptureComplete() const override
    {
        return !_captureRequested;
    }

    // Request that the viewer be closed after the next frame is rendered.
    void requestExit() override
    {
        _exitRequested = true;
    }

    // Return the active image handler.
    mx::ImageHandlerPtr getImageHandler() const override
    {
        return _imageHandler;
    }

    // Return the set of files referenced by XInclude.
    const mx::StringSet& getXincludeFiles() const override
    {
        return _xincludeFiles;
    }

    // Return true if the given node definition is supported by this backend.
    bool isNodeDefSupported(const mx::NodeDefPtr& nodeDef) override;

    // Update a uniform in the currently selected material.
    void modifyUniform(const std::string& name, mx::ValuePtr value) override;

    // Set the render pass to display (e.g. "combined", "albedo").
    void setRenderPass(const std::string& name) override;

    // Return the render pass currently being displayed.
    std::string getRenderPass() const override
    {
        return _renderPass;
    }

    // Enable or disable the OpenImageDenoise (CPU) denoiser.
    void setDenoise(bool enabled) override;

    // Set the number of samples to render before denoising begins.
    void setDenoiseStartSample(int samples) override;

    // Enable or disable adaptive (noise-threshold) sampling.
    void setAdaptiveSampling(bool enabled) override;

    // Set the adaptive sampling noise threshold (lower is stricter).
    void setAdaptiveThreshold(float threshold) override;

    // Set the minimum number of samples rendered with adaptive sampling.
    void setAdaptiveMinSamples(int samples) override;

    // Set the maximum number of samples rendered.
    void setMaxSamples(int samples) override;

    // Set a multiplier for the environment lighting intensity.
    void setLightIntensity(float intensity) override;

    // Pause/resume the Cycles session when this backend is deactivated/activated.
    void setActive(bool active) override;

  private:
    void initContext(mx::GenContext& context);
    void generateOsl(mx::TypedElementPtr typedElem);
    void rebuildMaterial();
    void buildScene();
    bool buildMesh(ccl::Scene* scene);
    bool buildEnvironment(ccl::Scene* scene);
    void buildFallbackSphere(ccl::Scene* scene);
    void applyDisplayPass(ccl::Scene* scene);
    void applyRenderSettings(ccl::Scene* scene);
    void applyLightIntensity();
    bool isDenoiseActive() const;
    void updateCamera();
    void restartRender();
    void uploadFrame(const std::vector<ccl::half4>& pixels, int width, int height);
    mx::Vector3 computeCameraEye() const;
    mx::Vector3 computeCameraUp() const;

    // Document management
    mx::DocumentPtr _document;
    mx::DocumentPtr _stdLib;
    mx::FileSearchPath _searchPath;
    mx::StringSet _xincludeFiles;

    // Scene resources.
    mx::FilePath _meshFilename;
    mx::FilePath _envRadianceFilename;
    mx::GeometryHandlerPtr _geometryHandler;

    // MaterialX OSL shader generation.
    std::unique_ptr<mx::GenContext> _genContext;
    mx::TypedElementPtr _currentElement;
    std::string _oslSource;
    std::string _oslOutputName;
    bool _materialDirty = false;
    bool _materialImmediate = false;
    double _materialDirtyTime = 0.0;
    int _materialVersion = 0;

    // Cycles session and scene.
    std::unique_ptr<ccl::Session> _session;
    CyclesCaptureDisplayDriver* _displayDriver = nullptr;
    ccl::Object* _object = nullptr;
    std::string _renderPass = "combined";

    // Denoising and sampling settings. The OpenImageDenoise CPU denoiser is
    // used when available; adaptive sampling stops early once the noise
    // threshold is met, after which Cycles always performs a final denoise.
    bool _denoise = true;
    bool _denoiseSupported = true;
    int _denoiseStartSample = 16;
    bool _adaptiveSampling = false;
    float _adaptiveThreshold = 0.01f;
    int _adaptiveMinSamples = 0;
    int _maxSamples = 4096;

    // Environment lighting intensity multiplier.
    float _lightIntensity = 0.75f;

    // Environment background nodes, split by ray type: the sharp radiance map for
    // specular/transmission rays and the blurred irradiance map for diffuse
    // rays. Both are scaled live by the light-intensity multiplier.
    ccl::BackgroundNode* _environmentBackground = nullptr;
    float _baseEnvironmentStrength = 1.0f;
    ccl::BackgroundNode* _irradianceBackground = nullptr;
    float _baseIrradianceStrength = 1.0f;

    // Resource handlers.
    mx::ImageHandlerPtr _imageHandler;
    mx::ImagePtr _image;
    mx::CameraPtr _viewCamera;
    unsigned int _textureID = 0;

    // Interactive camera state. The base camera matches the default scene
    // template and is orbited by the arcball stored in _viewCamera.
    mx::Vector3 _cameraPosition;
    mx::Vector3 _cameraTarget;
    mx::Vector3 _cameraUp;
    float _cameraFov = 45.0f;
    float _cameraZoom = 1.0f;
    mx::Vector3 _userTranslation;
    mx::Vector3 _userTranslationStart;
    bool _userTranslationActive = false;
    mx::Vector2 _userTranslationPixel;
    bool _cameraDirty = false;
    bool _cameraInteracting = false;
    double _lastCameraChangeTime = 0.0;

    // Frame capture.
    bool _captureRequested = false;
    double _captureRequestTime = 0.0;
    mx::FilePath _captureFilename;
    bool _exitRequested = false;

    // Size of the render buffer the session was last configured with.
    int _sessionWidth = 0;
    int _sessionHeight = 0;

    // Whether this backend is the one currently being displayed. Inactive
    // sessions are paused so they do not consume CPU in the background.
    bool _active = false;
};

#endif // MATERIALX_BUILD_RENDER_CYCLES

#endif
