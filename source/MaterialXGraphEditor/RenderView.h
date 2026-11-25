//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_RENDERVIEW_H
#define MATERIALX_RENDERVIEW_H

#include <MaterialXGraphEditor/RenderPipeline.h>
#include <MaterialXGraphEditor/RenderTypes.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

class RenderView;
using RenderViewPtr = std::shared_ptr<RenderView>;

/// @class RenderView
/// Backend-neutral view used by the graph editor.
///
/// The view owns the document, camera interaction, render settings and the
/// presentation surface.  Rendering itself is delegated to a RenderPipeline,
/// so adding a renderer does not require a new view subclass.
class RenderView
{
  public:
    RenderView(mx::DocumentPtr doc,
               mx::DocumentPtr stdLib,
               const std::string& meshFilename,
               const std::string& envRadianceFilename,
               const mx::FileSearchPath& searchPath,
               int viewWidth,
               int viewHeight);
    ~RenderView() = default;

    /// Return the names of all render backends available in this build.
    static std::vector<std::string> getAvailableBackends();

    /// Set the active render backend, creating or reusing its pipeline.
    void setRenderBackend(const std::string& backendName);

    /// Return the name of the active render backend.
    std::string getBackendName() const;

    /// Return the capabilities of the active backend.
    RenderCapabilities getCapabilities() const;

    /// Initialize the active backend.
    void initialize();

    /// Set the document to be rendered.
    void setDocument(mx::DocumentPtr document);

    /// Update materials for the given element, or the default element if null.
    void updateMaterials(mx::TypedElementPtr typedElem);

    /// Load a mesh from the given filename.
    void loadMesh(const mx::FilePath& filename);

    /// Render the current frame and update the display texture.
    void drawContents();

    /// Return the OpenGL texture ID of the most recently rendered frame.
    unsigned int getRenderTextureId() const;

    /// Called before the rendered frame is drawn by ImGui.
    void beginFrameDisplay();

    /// Called after the rendered frame is drawn by ImGui.
    void endFrameDisplay();

    /// Return the camera used to view the rendered scene.
    mx::CameraPtr getViewCamera()
    {
        return _viewCamera;
    }

    /// Return the pixel ratio of the render view.
    float getPixelRatio() const
    {
        return _pixelRatio;
    }

    /// Set the view width.
    void setViewWidth(int width)
    {
        _viewWidth = width;
    }

    /// Return the view width.
    int getViewWidth() const
    {
        return _viewWidth;
    }

    /// Set the view height.
    void setViewHeight(int height)
    {
        _viewHeight = height;
    }

    /// Return the view height.
    int getViewHeight() const
    {
        return _viewHeight;
    }

    /// Pass a mouse button event to the render view.
    void setMouseButtonEvent(int button, bool down, mx::Vector2 pos);

    /// Pass a mouse motion event to the render view.
    void setMouseMotionEvent(mx::Vector2 pos);

    /// Pass a key event to the render view.
    void setKeyEvent(int key);

    /// Pass a scroll event to the render view.
    void setScrollEvent(float scrollY);

    /// Request a capture of the current frame, writing it to the given filename.
    void requestFrameCapture(const mx::FilePath& filename);

    /// Return true once a requested frame capture has been written, or if no
    /// capture is pending.
    bool isFrameCaptureComplete() const;

    /// Request that the viewer be closed after the next frame is rendered.
    void requestExit();

    /// Return the active image handler.
    mx::ImageHandlerPtr getImageHandler() const;

    /// Return the set of files referenced by XInclude.
    const mx::StringSet& getXincludeFiles() const;

    /// Return true if the given node definition is supported by this view.
    bool isNodeDefSupported(const mx::NodeDefPtr& nodeDef);

    /// Update a uniform in the currently selected material.
    void modifyUniform(const std::string& name, mx::ValuePtr value);

    /// Set the render pass to display (e.g. "combined", "albedo").
    void setRenderPass(const std::string& name);

    /// Return the name of the render pass currently being displayed.
    std::string getRenderPass() const
    {
        return _settings.renderPass;
    }

    /// Enable or disable a final denoising step.
    void setDenoise(bool enabled);

    /// Set the number of samples to render before denoising begins.
    void setDenoiseStartSample(int samples);

    /// Set the number of median-filter denoising passes.
    void setDenoisePasses(int passes);

    /// Enable or disable adaptive (noise-threshold) sampling.
    void setAdaptiveSampling(bool enabled);

    /// Set the adaptive sampling noise threshold (lower is stricter).
    void setAdaptiveThreshold(float threshold);

    /// Set the minimum number of samples rendered when adaptive sampling is used.
    void setAdaptiveMinSamples(int samples);

    /// Set the maximum number of samples rendered.
    void setMaxSamples(int samples);

    /// Set a multiplier for the environment lighting intensity.
    void setLightIntensity(float intensity);

    /// Return the current backend-neutral render settings.
    const RenderSettings& getSettings() const
    {
        return _settings;
    }

    /// Notify the view that it has become the active backend.
    void setActive(bool active);

    /// Return true if a material compilation is in progress.
    bool getMaterialCompilation() const
    {
        return _materialCompilation;
    }

    /// Set whether a material compilation is in progress.
    void setMaterialCompilation(bool mat)
    {
        _materialCompilation = mat;
    }

    /// Return the current frame index.
    unsigned int getFrame() const
    {
        return _frame;
    }

    /// Set the current frame index.
    void setFrame(unsigned int frame)
    {
        _frame = frame;
        if (_activePipeline)
        {
            _activePipeline->setFrame(frame);
        }
    }

  private:
    RenderPipelinePtr createPipeline(const std::string& backendName);
    void applySettings(RenderPipeline& pipeline);
    void applyCamera(RenderPipeline& pipeline);
    mx::Vector3 computeCameraEye() const;
    mx::Vector3 computeCameraUp() const;

  private:
    RenderPipelinePtr _activePipeline;
    std::map<std::string, RenderPipelinePtr> _pipelines;
    std::map<std::string, mx::DocumentPtr> _pipelineDocs;

    mx::DocumentPtr _document;
    mx::DocumentPtr _stdLib;
    mx::FileSearchPath _searchPath;
    mx::FilePath _meshFilename;
    mx::FilePath _envRadianceFilename;

    mx::CameraPtr _viewCamera;
    RenderCameraState _cameraState;
    RenderSettings _settings;

    int _viewWidth = 0;
    int _viewHeight = 0;
    float _pixelRatio = 1.0f;
    bool _materialCompilation = false;
    unsigned int _frame = 0;
    bool _exitRequested = false;

    // Interactive camera state.
    mx::Vector3 _userTranslationStart;
    bool _userTranslationActive = false;
    mx::Vector2 _userTranslationPixel;
};

#endif
