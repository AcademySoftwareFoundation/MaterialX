//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_RENDERVIEWBASE_H
#define MATERIALX_RENDERVIEWBASE_H

#include <MaterialXCore/Types.h>
#include <MaterialXCore/Value.h>
#include <MaterialXFormat/File.h>
#include <MaterialXRender/Camera.h>
#include <MaterialXRender/ImageHandler.h>

#include <memory>

namespace mx = MaterialX;

class RenderViewBase;
using RenderViewBasePtr = std::shared_ptr<RenderViewBase>;

/// @class RenderViewBase
/// Abstract interface for render views embedded in the graph editor.
///
/// Implementations provide a rendered frame as an OpenGL texture that can be
/// displayed with ImGui, along with document, material, geometry, and camera
/// management.  Backend-specific details (shader generation, graphics API,
/// resources) are hidden behind this interface so that alternative renderers
/// (e.g. a ray tracer) can be plugged in.
class RenderViewBase
{
  public:
    virtual ~RenderViewBase() = default;

    /// Return the name of this render backend (e.g. "GLSL").
    virtual std::string getBackendName() const = 0;

    /// Initialize the render view for rendering.
    virtual void initialize() = 0;

    /// Set the document to be rendered.
    virtual void setDocument(mx::DocumentPtr document) = 0;

    /// Update materials for the given element, or the default element if null.
    virtual void updateMaterials(mx::TypedElementPtr typedElem) = 0;

    /// Load a mesh from the given filename.
    virtual void loadMesh(const mx::FilePath& filename) = 0;

    /// Render the current frame and update the display texture.
    virtual void drawContents() = 0;

    /// Return the OpenGL texture ID of the most recently rendered frame.
    virtual unsigned int getRenderTextureId() const = 0;

    /// Called before the rendered frame is drawn by ImGui.
    virtual void beginFrameDisplay() { }

    /// Called after the rendered frame is drawn by ImGui.
    virtual void endFrameDisplay() { }

    /// Return the camera used to view the rendered scene.
    virtual mx::CameraPtr getViewCamera() = 0;

    /// Return the pixel ratio of the render view.
    virtual float getPixelRatio() const = 0;

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
    virtual void setMouseButtonEvent(int button, bool down, mx::Vector2 pos) = 0;

    /// Pass a mouse motion event to the render view.
    virtual void setMouseMotionEvent(mx::Vector2 pos) = 0;

    /// Pass a key event to the render view.
    virtual void setKeyEvent(int key) = 0;

    /// Pass a scroll event to the render view.
    virtual void setScrollEvent(float scrollY) = 0;

    /// Request a capture of the current frame, writing it to the given filename.
    virtual void requestFrameCapture(const mx::FilePath& filename) = 0;

    /// Return true once a requested frame capture has been written, or if no
    /// capture is pending. Allows asynchronous backends to hold the main loop
    /// until the capture is complete.
    virtual bool isFrameCaptureComplete() const
    {
        return true;
    }

    /// Request that the viewer be closed after the next frame is rendered.
    virtual void requestExit() = 0;

    /// Return the active image handler.
    virtual mx::ImageHandlerPtr getImageHandler() const = 0;

    /// Return the set of files referenced by XInclude.
    virtual const mx::StringSet& getXincludeFiles() const = 0;

    /// Return true if the given node definition is supported by this render view.
    virtual bool isNodeDefSupported(const mx::NodeDefPtr& nodeDef) = 0;

    /// Update a uniform in the currently selected material.
    virtual void modifyUniform(const std::string& name, mx::ValuePtr value) = 0;

    /// Set the render pass to display (e.g. "combined", "albedo"). Backends
    /// that do not support multiple passes may ignore this.
    virtual void setRenderPass(const std::string& /*name*/) { }

    /// Return the name of the render pass currently being displayed.
    virtual std::string getRenderPass() const
    {
        return "combined";
    }

    /// Enable or disable a final denoising step. Backends without a denoiser
    /// may ignore this.
    virtual void setDenoise(bool /*enabled*/) { }

    /// Set the number of samples to render before denoising begins.
    virtual void setDenoiseStartSample(int /*samples*/) { }

    /// Enable or disable adaptive (noise-threshold) sampling.
    virtual void setAdaptiveSampling(bool /*enabled*/) { }

    /// Set the adaptive sampling noise threshold (lower is stricter).
    virtual void setAdaptiveThreshold(float /*threshold*/) { }

    /// Set the minimum number of samples rendered when adaptive sampling is used.
    virtual void setAdaptiveMinSamples(int /*samples*/) { }

    /// Set the maximum number of samples rendered.
    virtual void setMaxSamples(int /*samples*/) { }

    /// Set a multiplier for the direct lighting (light rig) intensity.
    virtual void setLightIntensity(float /*intensity*/) { }

    /// Notify the view that it has become the active backend (true) or has been
    /// switched away from (false). Asynchronous backends can use this to pause
    /// rendering while they are not being displayed.
    virtual void setActive(bool /*active*/) { }

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
    }

  protected:
    int _viewWidth = 0;
    int _viewHeight = 0;
    bool _materialCompilation = false;
    unsigned int _frame = 0;
};

#endif
