//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_OSLTESTRENDERPIPELINE_H
#define MATERIALX_OSLTESTRENDERPIPELINE_H

#ifdef MATERIALX_BUILD_RENDER_OSL_TESTRENDER

#include <MaterialXGraphEditor/RenderPipeline.h>
#include <MaterialXGraphEditor/GraphEditorOslGenerator.h>

#include <MaterialXRender/GeometryHandler.h>

#include <OSL/testrender.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace mx = MaterialX;

/// @class OslTestRenderPipeline
/// OSL testrender preview backend for the graph editor.
///
/// The pipeline generates OSL with the same MaterialX shader generator used by
/// the Cycles backend, compiles it with liboslcomp, and renders incrementally
/// with the reusable libtestrender CPU path tracer.  Rendering runs on a
/// worker thread and publishes completed sample passes to the main thread.
class OslTestRenderPipeline : public RenderPipeline
{
  public:
    OslTestRenderPipeline(mx::DocumentPtr doc,
                          mx::DocumentPtr stdLib,
                          const std::string& meshFilename,
                          const std::string& envRadianceFilename,
                          const mx::FileSearchPath& searchPath,
                          int viewWidth,
                          int viewHeight,
                          const std::string& renderPass = "combined");
    ~OslTestRenderPipeline() override;

    std::string getName() const override
    {
        return "OSL (testrender)";
    }

    RenderCapabilities getCapabilities() const override;
    void initialize() override;
    void setDocument(mx::DocumentPtr document) override;
    void updateMaterials(mx::TypedElementPtr typedElem) override;
    void loadMesh(const mx::FilePath& filename) override;
    void setSettings(const RenderSettings& settings) override;
    void resize(int width, int height) override;
    void setCamera(mx::CameraPtr camera, const RenderCameraState& state) override;
    void render() override;
    unsigned int getColorTexture() const override;
    void requestFrameCapture(const mx::FilePath& filename) override;
    bool isFrameCaptureComplete() const override;
    bool isBusy() const override;
    bool isNodeDefSupported(const mx::NodeDefPtr& nodeDef) override;
    void modifyUniform(const std::string& name, mx::ValuePtr value) override;
    void setActive(bool active) override;
    mx::ImageHandlerPtr getImageHandler() const override;
    const mx::StringSet& getXincludeFiles() const override;

  private:
    void generateOsl(mx::TypedElementPtr typedElem);
    void uploadFrame();
    void workerLoop();
    void notifyWorker();
    bool buildMesh(OSL::TestRenderMesh& mesh, const mx::FilePath& meshFilename);
    bool createSession(const RenderSettings& settings,
                       const RenderCameraState& cameraState, int width,
                       int height, const std::string& oslSource,
                       const std::string& oslOutput,
                       const std::string& meshFilename, std::string& error);
    void stopWorker();
    mx::Vector3 computeCameraEye(const RenderCameraState& state) const;
    mx::Vector3 computeCameraUp(const RenderCameraState& state) const;

    // Document / shader generation (main thread).
    mx::DocumentPtr _document;
    mx::DocumentPtr _stdLib;
    mx::FileSearchPath _searchPath;
    mx::StringSet _xincludeFiles;
    std::unique_ptr<GraphEditorOslGenerator> _oslGenerator;
    mx::TypedElementPtr _currentElement;
    bool _materialDirty = false;
    bool _materialImmediate = false;
    double _materialDirtyTime = 0.0;

    // Pending shader source handed to the worker.
    std::mutex _mutex;
    std::condition_variable _cv;
    std::string _pendingOslSource;
    std::string _pendingOslOutput;
    unsigned long long _pendingMaterialVersion = 0;

    // Worker state.
    std::thread _worker;
    std::atomic<bool> _stop { false };
    std::atomic<bool> _active { false };
    std::atomic<bool> _busy { true };
    std::atomic<bool> _prepared { false };
    std::atomic<bool> _hasSession { false };
    std::atomic<bool> _setupFailed { false };
    std::atomic<int> _samplesDone { 0 };
    std::atomic<unsigned long long> _seenMaterialVersion { 0 };
    bool _cameraDirty = false;
    bool _passDirty = false;
    bool _settingsDirty = false;
    bool _resetRequested = false;
    bool _sceneDirty = false;

    // Session ownership is shared between the worker and setActive().
    std::mutex _sessionMutex;
    std::unique_ptr<OSL::TestRenderSession> _session;

    // Desired state.
    mx::FilePath _meshFilename;
    mx::FilePath _envRadianceFilename;
    std::string _renderPass = "combined";
    RenderSettings _settings;
    RenderCameraState _cameraState;
    mx::CameraPtr _viewCamera;
    mx::Vector3 _cameraEye;
    mx::Vector3 _cameraUp;
    unsigned int _cameraRevision = 0;
    int _viewWidth = 0;
    int _viewHeight = 0;

    // Published CPU frame.
    mutable std::mutex _frameMutex;
    std::vector<float> _frame;
    int _frameWidth = 0;
    int _frameHeight = 0;
    unsigned long long _frameVersion = 0;
    unsigned long long _uploadedFrameVersion = 0;

    // Presentation and capture (main thread).
    mx::ImageHandlerPtr _imageHandler;
    mx::ImagePtr _image;
    unsigned int _textureID = 0;
    bool _captureRequested = false;
    mx::FilePath _captureFilename;
};

#endif // MATERIALX_BUILD_RENDER_OSL_TESTRENDER

#endif // MATERIALX_OSLTESTRENDERPIPELINE_H
