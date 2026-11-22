//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_RENDERPIPELINE_H
#define MATERIALX_RENDERPIPELINE_H

#include <MaterialXGraphEditor/RenderTypes.h>

#include <memory>

class RenderPipeline;
using RenderPipelinePtr = std::shared_ptr<RenderPipeline>;

/// @class RenderPipeline
/// Abstract renderer backend used by the graph editor.
///
/// A pipeline owns the backend-specific rendering state (graphics API
/// resources, ray-tracing session, framebuffers, materials) but knows nothing
/// about ImGui, the graph UI, or input handling.  A single RenderView owns the
/// active pipeline and handles presentation and camera interaction.
class RenderPipeline
{
  public:
    virtual ~RenderPipeline() = default;

    /// Return the backend name (e.g. "GLSL" or "Cycles").
    virtual std::string getName() const = 0;

    /// Return the controls and output modes supported by this backend.
    virtual RenderCapabilities getCapabilities() const = 0;

    /// Initialize backend resources.  Must be called after construction.
    virtual void initialize() = 0;

    /// Set the document to be rendered.
    virtual void setDocument(mx::DocumentPtr document) = 0;

    /// Update materials for the given element, or the default element if null.
    virtual void updateMaterials(mx::TypedElementPtr typedElem) = 0;

    /// Load a mesh from the given filename.
    virtual void loadMesh(const mx::FilePath& filename) = 0;

    /// Apply backend-neutral render settings.  Unsupported values are ignored.
    virtual void setSettings(const RenderSettings& settings) = 0;

    /// Update the size of the rendered image.
    virtual void resize(int width, int height) = 0;

    /// Set the shared camera and its interactive state.
    virtual void setCamera(mx::CameraPtr camera, const RenderCameraState& state) = 0;

    /// Set the current animation frame index.
    virtual void setFrame(unsigned int /*frame*/) { }

    /// Render the current frame into the pipeline's color texture.
    virtual void render() = 0;

    /// Return the OpenGL texture ID of the most recently rendered frame.
    virtual unsigned int getColorTexture() const = 0;

    /// Request a capture of the current frame, writing it to the given filename.
    virtual void requestFrameCapture(const mx::FilePath& filename) = 0;

    /// Return true once a requested frame capture has been written, or if no
    /// capture is pending.
    virtual bool isFrameCaptureComplete() const = 0;

    /// Return true if the pipeline is still preparing a material for display.
    virtual bool isBusy() const
    {
        return false;
    }

    /// Return true if the given node definition is supported by this backend.
    virtual bool isNodeDefSupported(const mx::NodeDefPtr& nodeDef) = 0;

    /// Update a uniform in the currently selected material.
    virtual void modifyUniform(const std::string& name, mx::ValuePtr value) = 0;

    /// Notify the pipeline that it has become active (true) or inactive (false).
    virtual void setActive(bool /*active*/) { }

    /// Return the active image handler.
    virtual mx::ImageHandlerPtr getImageHandler() const = 0;

    /// Return the set of files referenced by XInclude.
    virtual const mx::StringSet& getXincludeFiles() const = 0;
};

#endif
