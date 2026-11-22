//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_CYCLESRENDERPIPELINE_H
#define MATERIALX_CYCLESRENDERPIPELINE_H

#ifdef MATERIALX_BUILD_RENDER_CYCLES

#include <MaterialXGraphEditor/RenderPipeline.h>

#include <MaterialXRender/GeometryHandler.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenOsl/OslShaderGenerator.h>

#include "session/display_driver.h"

#include <memory>
#include <vector>

namespace mx = MaterialX;

class CyclesRenderPipeline;
using CyclesRenderPipelinePtr = std::shared_ptr<CyclesRenderPipeline>;

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

/// @class CyclesRenderPipeline
/// Cycles render pipeline embedded in the graph editor.
///
/// Renders an interactive path-traced preview using the Cycles renderer and
/// exposes the result to RenderView as an OpenGL texture.
class CyclesRenderPipeline : public RenderPipeline
{
  public:
    CyclesRenderPipeline(mx::DocumentPtr doc,
                     mx::DocumentPtr stdLib,
                     const std::string& meshFilename,
                     const std::string& envRadianceFilename,
                     const mx::FileSearchPath& searchPath,
                     int viewWidth,
                     int viewHeight,
                     const std::string& renderPass = "combined");
    ~CyclesRenderPipeline();

    // Return the name of this render backend.
    std::string getBackendName() const
    {
        return "Cycles";
    }

    std::string getName() const override
    {
        return getBackendName();
    }

    RenderCapabilities getCapabilities() const override
    {
        RenderCapabilities caps;
        caps.passes = { { "combined", "Combined" }, { "albedo", "Albedo" } };
        caps.denoising = true;
        caps.adaptiveSampling = true;
        caps.lightIntensity = true;
        caps.asynchronous = true;
        return caps;
    }

    // Initialize the viewer for rendering.
    void initialize() override;

    void setSettings(const RenderSettings& settings) override
    {
        setRenderPass(settings.renderPass);
        setDenoise(settings.denoise);
        setDenoiseStartSample(settings.denoiseStartSample);
        setAdaptiveSampling(settings.adaptiveSampling);
        setAdaptiveThreshold(settings.adaptiveThreshold);
        setAdaptiveMinSamples(settings.adaptiveMinSamples);
        setMaxSamples(settings.maxSamples);
        setLightIntensity(settings.lightIntensity);
    }

    void resize(int width, int height) override
    {
        _viewWidth = width;
        _viewHeight = height;
    }

    void setCamera(mx::CameraPtr camera, const RenderCameraState& state) override
    {
        if (state.revision != _cameraRevision)
        {
            _cameraRevision = state.revision;
            _cameraDirty = true;
        }
        _viewCamera = camera;
        _cameraPosition = state.position;
        _cameraTarget = state.target;
        _cameraUp = state.up;
        _cameraFov = state.viewAngle;
        _cameraZoom = state.zoom;
        _userTranslation = state.userTranslation;
    }

    void render() override
    {
        drawContents();
    }

    unsigned int getColorTexture() const override
    {
        return getRenderTextureId();
    }

    void setDocument(mx::DocumentPtr document);
    void updateMaterials(mx::TypedElementPtr typedElem);
    void loadMesh(const mx::FilePath& filename);
    void drawContents();

    // Return the OpenGL texture ID of the most recently rendered frame.
    unsigned int getRenderTextureId() const
    {
        return _textureID;
    }

    // Request a capture of the current frame, writing it to the given filename.
    void requestFrameCapture(const mx::FilePath& filename);

    // Return true once the requested frame capture has been written.
    bool isFrameCaptureComplete() const
    {
        return !_captureRequested;
    }

    // Return the active image handler.
    mx::ImageHandlerPtr getImageHandler() const
    {
        return _imageHandler;
    }

    // Return the set of files referenced by XInclude.
    const mx::StringSet& getXincludeFiles() const
    {
        return _xincludeFiles;
    }

    // Return true if the given node definition is supported by this backend.
    bool isNodeDefSupported(const mx::NodeDefPtr& nodeDef);

    // Update a uniform in the currently selected material.
    void modifyUniform(const std::string& name, mx::ValuePtr value);

    // Set the render pass to display (e.g. "combined", "albedo").
    void setRenderPass(const std::string& name);

    // Return the render pass currently being displayed.
    std::string getRenderPass() const
    {
        return _renderPass;
    }

    // Enable or disable the OpenImageDenoise (CPU) denoiser.
    void setDenoise(bool enabled);

    // Set the number of samples to render before denoising begins.
    void setDenoiseStartSample(int samples);

    // Enable or disable adaptive (noise-threshold) sampling.
    void setAdaptiveSampling(bool enabled);

    // Set the adaptive sampling noise threshold (lower is stricter).
    void setAdaptiveThreshold(float threshold);

    // Set the minimum number of samples rendered with adaptive sampling.
    void setAdaptiveMinSamples(int samples);

    // Set the maximum number of samples rendered.
    void setMaxSamples(int samples);

    // Set a multiplier for the environment lighting intensity.
    void setLightIntensity(float intensity);

    // Pause/resume the Cycles session when this backend is deactivated/activated.
    void setActive(bool active);

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
    bool _cameraDirty = false;
    unsigned int _cameraRevision = 0;

    // Frame capture.
    bool _captureRequested = false;
    double _captureRequestTime = 0.0;
    mx::FilePath _captureFilename;

    // Size requested by the render view and the size the session was last
    // configured with.
    int _viewWidth = 0;
    int _viewHeight = 0;
    int _sessionWidth = 0;
    int _sessionHeight = 0;

    // Whether this backend is the one currently being displayed. Inactive
    // sessions are paused so they do not consume CPU in the background.
    bool _active = false;
};

#endif // MATERIALX_BUILD_RENDER_CYCLES

#endif
