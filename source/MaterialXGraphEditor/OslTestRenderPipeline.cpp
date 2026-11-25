//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifdef MATERIALX_BUILD_RENDER_OSL_TESTRENDER

#include <MaterialXGraphEditor/OslTestRenderPipeline.h>

#include <MaterialXRenderGlsl/GLTextureHandler.h>
#include <MaterialXRenderGlsl/External/Glad/glad.h>
#include <MaterialXRender/StbImageLoader.h>
#include <MaterialXRender/CgltfLoader.h>
#include <MaterialXRender/TinyObjLoader.h>
#include <MaterialXRender/Mesh.h>
#include <MaterialXRender/Util.h>

#include <MaterialXGenShader/Util.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>

namespace
{

const double MATERIAL_SETTLE_SECONDS = 0.5;
const int TESTRENDER_PREVIEW_BOUNCES = 8;

double currentTimeSeconds()
{
    return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

void appendSphere(OSL::TestRenderMesh& mesh)
{
    const int segments = 64;
    const int rings = 32;
    const float pi = std::acos(-1.0f);
    const float radius = 1.0f;

    mesh.positions.clear();
    mesh.normals.clear();
    mesh.texcoords.clear();
    mesh.triangles.clear();

    for (int y = 0; y <= rings; ++y)
    {
        const float v = (float) y / (float) rings;
        const float theta = v * pi;
        for (int x = 0; x <= segments; ++x)
        {
            const float u = (float) x / (float) segments;
            const float phi = u * 2.0f * pi;
            const float nx = std::sin(theta) * std::cos(phi);
            const float ny = std::cos(theta);
            const float nz = std::sin(theta) * std::sin(phi);
            mesh.positions.insert(mesh.positions.end(),
                                  { radius * nx, radius * ny, radius * nz });
            mesh.normals.insert(mesh.normals.end(), { nx, ny, nz });
            mesh.texcoords.insert(mesh.texcoords.end(), { u, 1.0f - v });
        }
    }

    const int rowStride = segments + 1;
    for (int y = 0; y < rings; ++y)
    {
        for (int x = 0; x < segments; ++x)
        {
            const int a = y * rowStride + x;
            const int b = a + rowStride;
            mesh.triangles.insert(mesh.triangles.end(),
                                  { a, b, a + 1, a + 1, b, b + 1 });
        }
    }
}

} // anonymous namespace

OslTestRenderPipeline::OslTestRenderPipeline(mx::DocumentPtr doc,
                                             mx::DocumentPtr stdLib,
                                             const std::string& meshFilename,
                                             const std::string& envRadianceFilename,
                                             const mx::FileSearchPath& searchPath,
                                             int viewWidth,
                                             int viewHeight,
                                             const std::string& renderPass) :
    _document(doc),
    _stdLib(stdLib),
    _searchPath(searchPath),
    _oslGenerator(std::make_unique<GraphEditorOslGenerator>(nullptr, searchPath)),
    _meshFilename(meshFilename),
    _envRadianceFilename(envRadianceFilename),
    _renderPass(renderPass),
    _viewWidth(viewWidth),
    _viewHeight(viewHeight)
{
    mx::FileSearchPath localSearchPath = _searchPath;
    localSearchPath.append(mx::FilePath::getCurrentPath());
    _meshFilename = localSearchPath.find(_meshFilename);
    _envRadianceFilename = localSearchPath.find(_envRadianceFilename);
    _settings.renderPass = renderPass;
    setDocument(doc);
}

OslTestRenderPipeline::~OslTestRenderPipeline()
{
    stopWorker();
    if (_imageHandler && _image)
    {
        _imageHandler->releaseRenderResources(_image);
    }
}

RenderCapabilities OslTestRenderPipeline::getCapabilities() const
{
    RenderCapabilities caps;
    caps.passes = { { "combined", "Combined" },
                    { "albedo", "Albedo" },
                    { "normal", "Normal" },
                    { "uv", "UV" } };
    caps.denoising = false;
    caps.adaptiveSampling = false;
    caps.sampling = true;
    caps.lightIntensity = true;
    caps.uniformUpdates = false;
    caps.meshReload = true;
    caps.asynchronous = true;
    return caps;
}

void OslTestRenderPipeline::initialize()
{
    _imageHandler = mx::GLTextureHandler::create(mx::StbImageLoader::create());
    _imageHandler->setSearchPath(_searchPath);

    if (_document)
    {
        generateOsl(_currentElement);
    }

    _stop = false;
    _active = false;
    _busy = true;
    _worker = std::thread(&OslTestRenderPipeline::workerLoop, this);
}

void OslTestRenderPipeline::stopWorker()
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _stop = true;
        _active = false;
    }
    _cv.notify_all();

    {
        std::lock_guard<std::mutex> lock(_sessionMutex);
        if (_session)
        {
            _session->cancel();
        }
    }

    if (_worker.joinable())
    {
        _worker.join();
    }

    {
        std::lock_guard<std::mutex> lock(_sessionMutex);
        _session.reset();
    }
    _hasSession = false;
}

void OslTestRenderPipeline::notifyWorker()
{
    _cv.notify_one();
}

void OslTestRenderPipeline::setDocument(mx::DocumentPtr document)
{
    _document = document;
    _currentElement = nullptr;
    if (_oslGenerator)
    {
        _oslGenerator->setDocument(document);
    }
    _materialDirty = true;
    _materialImmediate = true;
    _materialDirtyTime = currentTimeSeconds();
    _setupFailed = false;
}

void OslTestRenderPipeline::updateMaterials(mx::TypedElementPtr typedElem)
{
    if (!typedElem && _document)
    {
        const std::vector<mx::TypedElementPtr> elements = mx::findRenderableElements(_document);
        if (!elements.empty())
        {
            typedElem = elements[0];
        }
    }
    _currentElement = typedElem;
    _materialDirty = true;
    _materialImmediate = true;
    _materialDirtyTime = currentTimeSeconds();
    _setupFailed = false;
}

void OslTestRenderPipeline::modifyUniform(const std::string& /*name*/, mx::ValuePtr /*value*/)
{
    _materialDirty = true;
    _materialImmediate = false;
    _materialDirtyTime = currentTimeSeconds();
}

void OslTestRenderPipeline::loadMesh(const mx::FilePath& filename)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _meshFilename = filename;
        _sceneDirty = true;
        _setupFailed = false;
    }
    notifyWorker();
}

void OslTestRenderPipeline::setSettings(const RenderSettings& settings)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _settings = settings;
        if (settings.renderPass != _renderPass)
        {
            _renderPass = settings.renderPass;
            _passDirty = true;
        }
        _settingsDirty = true;
        _setupFailed = false;
    }
    notifyWorker();
}

void OslTestRenderPipeline::resize(int width, int height)
{
    bool changed = false;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_viewWidth != width || _viewHeight != height)
        {
            _viewWidth = width;
            _viewHeight = height;
            _cameraDirty = true;
            _setupFailed = false;
            changed = true;
        }
    }
    if (changed)
    {
        notifyWorker();
    }
}

void OslTestRenderPipeline::setCamera(mx::CameraPtr camera, const RenderCameraState& state)
{
    if (!camera)
    {
        return;
    }
    bool changed = false;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _viewCamera = camera;
        if (state.revision != _cameraRevision)
        {
            _cameraRevision = state.revision;
            changed = true;
        }
        _cameraState = state;
        _cameraEye = computeCameraEye(state) + state.userTranslation;
        _cameraUp = computeCameraUp(state);
        if (changed)
        {
            _cameraDirty = true;
            _setupFailed = false;
        }
    }
    if (changed)
    {
        notifyWorker();
    }
}

mx::Vector3 OslTestRenderPipeline::computeCameraEye(const RenderCameraState& state) const
{
    mx::Matrix44 invArcball = _viewCamera->arcballMatrix().getInverse();
    mx::Vector3 offset = invArcball.transformVector(state.position - state.target);
    offset = offset * (1.0f / state.zoom);
    return state.target + offset;
}

mx::Vector3 OslTestRenderPipeline::computeCameraUp(const RenderCameraState& state) const
{
    mx::Matrix44 invArcball = _viewCamera->arcballMatrix().getInverse();
    return invArcball.transformVector(state.up);
}

void OslTestRenderPipeline::generateOsl(mx::TypedElementPtr typedElem)
{
    if (!_oslGenerator)
    {
        return;
    }

    const GraphEditorOslGenerator::Result result = _oslGenerator->generate(typedElem);
    if (!result)
    {
        if (!_oslGenerator->getLastError().empty())
        {
            std::cerr << "OSL (testrender): failed to generate OSL: "
                      << _oslGenerator->getLastError() << std::endl;
        }
        return;
    }

    {
        std::lock_guard<std::mutex> lock(_mutex);
        _pendingOslSource = result.source;
        _pendingOslOutput = result.outputName;
        ++_pendingMaterialVersion;
        _setupFailed = false;
    }
    notifyWorker();
}

bool OslTestRenderPipeline::buildMesh(OSL::TestRenderMesh& mesh, const mx::FilePath& meshFilename)
{
    mesh = OSL::TestRenderMesh();
    if (meshFilename.isEmpty() || !meshFilename.exists())
    {
        appendSphere(mesh);
        return true;
    }

    mx::GeometryHandlerPtr geometryHandler = mx::GeometryHandler::create();
    geometryHandler->addLoader(mx::TinyObjLoader::create());
    geometryHandler->addLoader(mx::CgltfLoader::create());
    if (!geometryHandler->loadGeometry(meshFilename) || geometryHandler->getMeshes().empty())
    {
        appendSphere(mesh);
        return true;
    }

    const mx::Vector3 boxMax = geometryHandler->getMaximumBounds();
    const mx::Vector3 boxMin = geometryHandler->getMinimumBounds();
    const mx::Vector3 sphereCenter = (boxMax + boxMin) * 0.5f;
    const float radius = (sphereCenter - boxMin).getMagnitude();
    const float scale = (radius > 0.0f) ? (2.0f / radius) : 1.0f;

    for (const mx::MeshPtr& sourceMesh : geometryHandler->getMeshes())
    {
        mx::MeshStreamPtr positionStream = sourceMesh->getStream(mx::MeshStream::POSITION_ATTRIBUTE, 0);
        if (!positionStream)
        {
            continue;
        }

        mx::MeshStreamPtr normalStream = sourceMesh->getStream(mx::MeshStream::NORMAL_ATTRIBUTE, 0);
        mx::MeshStreamPtr texcoordStream = sourceMesh->getStream(mx::MeshStream::TEXCOORD_ATTRIBUTE, 0);

        const size_t vertexOffset = mesh.positions.size() / 3;
        const mx::MeshFloatBuffer& positionData = positionStream->getData();
        const size_t vertexCount = positionStream->getSize();
        for (size_t i = 0; i < vertexCount; ++i)
        {
            mesh.positions.insert(mesh.positions.end(),
                                  { (positionData[i * 3 + 0] - sphereCenter[0]) * scale,
                                    (positionData[i * 3 + 1] - sphereCenter[1]) * scale,
                                    (positionData[i * 3 + 2] - sphereCenter[2]) * scale });
        }

        if (normalStream && normalStream->getSize() == vertexCount)
        {
            const mx::MeshFloatBuffer& normalData = normalStream->getData();
            for (size_t i = 0; i < vertexCount; ++i)
            {
                mesh.normals.insert(mesh.normals.end(),
                                    { normalData[i * 3 + 0],
                                      normalData[i * 3 + 1],
                                      normalData[i * 3 + 2] });
            }
        }
        else
        {
            for (size_t i = 0; i < vertexCount; ++i)
            {
                mesh.normals.insert(mesh.normals.end(), { 0.0f, 0.0f, 1.0f });
            }
        }

        if (texcoordStream && texcoordStream->getSize() == vertexCount)
        {
            const mx::MeshFloatBuffer& texcoordData = texcoordStream->getData();
            for (size_t i = 0; i < vertexCount; ++i)
            {
                mesh.texcoords.insert(mesh.texcoords.end(),
                                      { texcoordData[i * 2 + 0],
                                        texcoordData[i * 2 + 1] });
            }
        }
        else
        {
            for (size_t i = 0; i < vertexCount; ++i)
            {
                mesh.texcoords.insert(mesh.texcoords.end(), { 0.0f, 0.0f });
            }
        }

        for (size_t partIndex = 0; partIndex < sourceMesh->getPartitionCount(); ++partIndex)
        {
            mx::MeshPartitionPtr part = sourceMesh->getPartition(partIndex);
            for (uint32_t index : part->getIndices())
            {
                mesh.triangles.push_back((int) (index + vertexOffset));
            }
        }
    }

    if (mesh.positions.empty() || mesh.triangles.empty())
    {
        appendSphere(mesh);
    }
    return true;
}

bool OslTestRenderPipeline::createSession(const RenderSettings& settings,
                                          const RenderCameraState& cameraState,
                                          int width,
                                          int height,
                                          const std::string& oslSource,
                                          const std::string& oslOutput,
                                          const std::string& meshFilename,
                                          std::string& error)
{
    auto session = std::make_unique<OSL::TestRenderSession>();

    OSL::TestRenderSettings renderSettings;
    renderSettings.maxBounces = TESTRENDER_PREVIEW_BOUNCES;
    renderSettings.rrDepth = 5;
    renderSettings.noJitter = false;
    renderSettings.stdoslPath = MATERIALX_OSL_STDOSL_PATH;
    renderSettings.textureSearchPath = _searchPath.asString();
    renderSettings.screenColor[0] = mx::DEFAULT_SCREEN_COLOR_LIN_REC709[0];
    renderSettings.screenColor[1] = mx::DEFAULT_SCREEN_COLOR_LIN_REC709[1];
    renderSettings.screenColor[2] = mx::DEFAULT_SCREEN_COLOR_LIN_REC709[2];

    mx::FilePath genOslIncludePath = _searchPath.find("libraries/stdlib/genosl/include");
    if (!genOslIncludePath.isEmpty())
    {
        renderSettings.includePaths.push_back(genOslIncludePath.asString());
    }
    for (size_t i = 0; i < _searchPath.size(); ++i)
    {
        renderSettings.includePaths.push_back(_searchPath[i].asString());
    }

    if (!session->initialize(renderSettings, error))
    {
        return false;
    }

    OSL::TestRenderMesh mesh;
    const bool meshOk = buildMesh(mesh, meshFilename);
    if (!meshOk)
    {
        error = "failed to build testrender mesh";
        return false;
    }

    if (!session->setScene(mesh, error))
    {
        return false;
    }

    OSL::TestRenderCamera camera;
    camera.eye[0] = cameraState.target[0];
    camera.eye[1] = cameraState.target[1];
    camera.eye[2] = cameraState.target[2];
    camera.width = width;
    camera.height = height;
    camera.fov = cameraState.viewAngle;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        camera.eye[0] = _cameraEye[0];
        camera.eye[1] = _cameraEye[1];
        camera.eye[2] = _cameraEye[2];
        camera.target[0] = cameraState.target[0] + cameraState.userTranslation[0];
        camera.target[1] = cameraState.target[1] + cameraState.userTranslation[1];
        camera.target[2] = cameraState.target[2] + cameraState.userTranslation[2];
        camera.up[0] = _cameraUp[0];
        camera.up[1] = _cameraUp[1];
        camera.up[2] = _cameraUp[2];
    }
    if (!session->setCamera(camera, error))
    {
        return false;
    }

    if (!session->setMaterialFromOsl(oslSource, "material_0", oslOutput, error))
    {
        std::cerr << "OSL (testrender): material setup failed: " << error << std::endl;
        return false;
    }

    if (!session->setRenderPass(settings.renderPass, error))
    {
        return false;
    }
    const std::string envMap = _envRadianceFilename.exists() ?
                                   _envRadianceFilename.asString() : std::string();
    if (!session->setEnvironmentMap(envMap, error))
    {
        return false;
    }
    if (!session->setLightIntensity(settings.lightIntensity, error))
    {
        return false;
    }

    if (!session->prepare(error))
    {
        std::cerr << "OSL (testrender): prepare failed: " << error << std::endl;
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(_sessionMutex);
        _session = std::move(session);
    }
    _hasSession = true;
    _prepared = true;
    _samplesDone = 0;
    _busy = false;
    return true;
}

void OslTestRenderPipeline::workerLoop()
{
    std::string appliedMesh = _meshFilename.asString();
    std::string appliedPass = _renderPass;
    float appliedIntensity = _settings.lightIntensity;
    int appliedWidth = 0;
    int appliedHeight = 0;
    unsigned long long appliedMaterialVersion = 0;

    while (true)
    {
        bool rebuildScene = false;
        bool cameraDirty = false;
        bool resetRequested = false;
        std::string oslSource;
        std::string oslOutput;
        unsigned long long materialVersion = 0;
        RenderSettings settings;
        RenderCameraState cameraState;
        mx::Vector3 cameraEye;
        mx::Vector3 cameraUp;
        int width = 0;
        int height = 0;
        std::string meshFilename;

        {
            std::unique_lock<std::mutex> lock(_mutex);
            auto ready = [&]()
            {
                if (_stop)
                {
                    return true;
                }
                if (!_active)
                {
                    return false;
                }
                const bool hasMaterial = !_pendingOslSource.empty();
                const bool needSession = (!_hasSession || _sceneDirty) && hasMaterial &&
                                         !_setupFailed.load();
                const bool hasPendingMaterial =
                    _seenMaterialVersion.load() != _pendingMaterialVersion;
                const bool needsSamples = _prepared.load() &&
                                          _samplesDone.load() < _settings.maxSamples;
                return needSession || _cameraDirty || _passDirty || _settingsDirty ||
                       _resetRequested || hasPendingMaterial || needsSamples;
            };

            _cv.wait(lock, ready);
            if (_stop)
            {
                break;
            }
            if (!_active)
            {
                continue;
            }

            rebuildScene = _sceneDirty;
            _sceneDirty = false;
            cameraDirty = _cameraDirty;
            _cameraDirty = false;
            _passDirty = false;
            _settingsDirty = false;
            resetRequested = _resetRequested;
            _resetRequested = false;
            oslSource = _pendingOslSource;
            oslOutput = _pendingOslOutput;
            materialVersion = _pendingMaterialVersion;
            settings = _settings;
            cameraState = _cameraState;
            cameraEye = _cameraEye;
            cameraUp = _cameraUp;
            width = _viewWidth;
            height = _viewHeight;
            meshFilename = _meshFilename.asString();
        }

        if (oslSource.empty())
        {
            _busy = true;
            continue;
        }

        if (!_hasSession || rebuildScene || meshFilename != appliedMesh)
        {
            _busy = true;
            std::string error;
            {
                std::lock_guard<std::mutex> lock(_sessionMutex);
                _session.reset();
            }
            _hasSession = false;
            if (createSession(settings, cameraState, width, height, oslSource,
                              oslOutput, meshFilename, error))
            {
                appliedMesh = meshFilename;
                appliedMaterialVersion = materialVersion;
                appliedWidth = width;
                appliedHeight = height;
                appliedPass = settings.renderPass;
                appliedIntensity = settings.lightIntensity;
                _seenMaterialVersion = materialVersion;
                _setupFailed = false;
            }
            else
            {
                _busy = true;
                _prepared = false;
                _setupFailed = true;
                std::cerr << "OSL (testrender): session setup failed: " << error
                          << std::endl;
                continue;
            }
        }

        OSL::TestRenderSession* session = nullptr;
        {
            std::lock_guard<std::mutex> lock(_sessionMutex);
            session = _session.get();
        }
        if (!session)
        {
            _hasSession = false;
            continue;
        }

        if (materialVersion != appliedMaterialVersion)
        {
            std::string error;
            const std::string shaderName = "material_" + std::to_string(materialVersion);
            if (session->setMaterialFromOsl(oslSource, shaderName, oslOutput, error))
            {
                appliedMaterialVersion = materialVersion;
                _seenMaterialVersion = materialVersion;
                _samplesDone = 0;
            }
            else
            {
                std::cerr << "OSL (testrender): material compile failed: " << error
                          << std::endl;
                _seenMaterialVersion = materialVersion;
            }
        }

        if (cameraDirty || width != appliedWidth || height != appliedHeight ||
            resetRequested)
        {
            OSL::TestRenderCamera camera;
            camera.eye[0] = cameraEye[0];
            camera.eye[1] = cameraEye[1];
            camera.eye[2] = cameraEye[2];
            camera.target[0] = cameraState.target[0] + cameraState.userTranslation[0];
            camera.target[1] = cameraState.target[1] + cameraState.userTranslation[1];
            camera.target[2] = cameraState.target[2] + cameraState.userTranslation[2];
            camera.up[0] = cameraUp[0];
            camera.up[1] = cameraUp[1];
            camera.up[2] = cameraUp[2];
            camera.fov = cameraState.viewAngle;
            camera.width = width;
            camera.height = height;
            std::string error;
            if (session->setCamera(camera, error))
            {
                appliedWidth = width;
                appliedHeight = height;
                _samplesDone = 0;
            }
            else
            {
                std::cerr << "OSL (testrender): camera update failed: " << error
                          << std::endl;
            }
        }

        if (settings.renderPass != appliedPass)
        {
            std::string error;
            if (session->setRenderPass(settings.renderPass, error))
            {
                appliedPass = settings.renderPass;
                _samplesDone = 0;
            }
            else
            {
                std::cerr << "OSL (testrender): render pass update failed: " << error
                          << std::endl;
            }
        }

        if (settings.lightIntensity != appliedIntensity)
        {
            std::string error;
            if (session->setLightIntensity(settings.lightIntensity, error))
            {
                appliedIntensity = settings.lightIntensity;
                _samplesDone = 0;
            }
            else
            {
                std::cerr << "OSL (testrender): light intensity update failed: "
                          << error << std::endl;
            }
        }

        if (!_prepared)
        {
            continue;
        }

        if (_samplesDone < settings.maxSamples)
        {
            std::string error;
            if (session->renderSamples(1, error, {}))
            {
                std::vector<float> frame;
                int frameWidth = 0;
                int frameHeight = 0;
                if (session->copyFrame(frame, frameWidth, frameHeight))
                {
                    std::lock_guard<std::mutex> lock(_frameMutex);
                    _frame = std::move(frame);
                    _frameWidth = frameWidth;
                    _frameHeight = frameHeight;
                    ++_frameVersion;
                }
                _samplesDone = session->samplesDone();
                _busy = false;
            }
            else
            {
                std::cerr << "OSL (testrender): render failed: " << error
                          << std::endl;
                _busy = true;
                _prepared = false;
                _setupFailed = true;
            }
        }
        else
        {
            _busy = false;
        }
    }

    _busy = false;
}

void OslTestRenderPipeline::render()
{
    if (_materialDirty)
    {
        const bool ready = _materialImmediate ||
                           (currentTimeSeconds() - _materialDirtyTime >= MATERIAL_SETTLE_SECONDS);
        if (ready)
        {
            _materialDirty = false;
            _materialImmediate = false;
            generateOsl(_currentElement);
        }
    }

    unsigned long long frameVersion = 0;
    {
        std::lock_guard<std::mutex> lock(_frameMutex);
        frameVersion = _frameVersion;
    }
    if (frameVersion != _uploadedFrameVersion)
    {
        uploadFrame();
        _uploadedFrameVersion = frameVersion;
    }

    if (_captureRequested && _image)
    {
        const int captureSamples = std::min(16, std::max(1, _settings.maxSamples));
        if (_samplesDone >= captureSamples)
        {
            mx::ImagePtr saveImage = _image;
            if (_captureFilename.getExtension() == "png" ||
                _captureFilename.getExtension() == "jpg" ||
                _captureFilename.getExtension() == "jpeg")
            {
                saveImage = mx::Image::create(_image->getWidth(), _image->getHeight(), 4,
                                              mx::Image::BaseType::UINT8);
                saveImage->createResourceBuffer();
                for (unsigned int y = 0; y < _image->getHeight(); ++y)
                {
                    for (unsigned int x = 0; x < _image->getWidth(); ++x)
                    {
                        mx::Color4 color = _image->getTexelColor(x, y);
                        for (int c = 0; c < 4; ++c)
                        {
                            color[c] = std::isfinite(color[c])
                                           ? std::clamp(color[c], 0.0f, 1.0f)
                                           : 0.0f;
                        }
                        const mx::Color3 srgb = mx::Color3(color[0], color[1], color[2])
                                                    .linearToSrgb();
                        color[0] = srgb[0];
                        color[1] = srgb[1];
                        color[2] = srgb[2];
                        saveImage->setTexelColor(x, y, color);
                    }
                }
            }
            if (_imageHandler->saveImage(_captureFilename, saveImage, true))
            {
                std::cout << "Wrote frame to disk: " << _captureFilename.asString() << std::endl;
            }
            else
            {
                std::cerr << "Failed to write frame to disk: "
                          << _captureFilename.asString() << std::endl;
            }
            _captureRequested = false;
        }
    }
}

void OslTestRenderPipeline::uploadFrame()
{
    std::vector<float> frame;
    int width = 0;
    int height = 0;
    {
        std::lock_guard<std::mutex> lock(_frameMutex);
        frame = _frame;
        width = _frameWidth;
        height = _frameHeight;
    }
    if (frame.empty() || width <= 0 || height <= 0 || !_imageHandler)
    {
        return;
    }

    if (!_image || (int) _image->getWidth() != width || (int) _image->getHeight() != height)
    {
        if (_image)
        {
            _imageHandler->releaseRenderResources(_image);
        }
        _image = mx::Image::create((unsigned int) width, (unsigned int) height, 4,
                                   mx::Image::BaseType::FLOAT);
        _image->createResourceBuffer();
    }

    float* destination = static_cast<float*>(_image->getResourceBuffer());
    for (size_t y = 0; y < (size_t) height; ++y)
    {
        const size_t sourceRow = (size_t) height - 1 - y;
        for (size_t x = 0; x < (size_t) width; ++x)
        {
            const size_t sourceIndex = (sourceRow * (size_t) width + x) * 3;
            const size_t destIndex = (y * (size_t) width + x) * 4;
            destination[destIndex + 0] = std::isfinite(frame[sourceIndex + 0])
                                             ? frame[sourceIndex + 0]
                                             : 0.0f;
            destination[destIndex + 1] = std::isfinite(frame[sourceIndex + 1])
                                             ? frame[sourceIndex + 1]
                                             : 0.0f;
            destination[destIndex + 2] = std::isfinite(frame[sourceIndex + 2])
                                             ? frame[sourceIndex + 2]
                                             : 0.0f;
            destination[destIndex + 3] = 1.0f;
        }
    }

    if (_imageHandler->createRenderResources(_image, false))
    {
        _textureID = _image->getResourceId();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

unsigned int OslTestRenderPipeline::getColorTexture() const
{
    return _textureID;
}

void OslTestRenderPipeline::requestFrameCapture(const mx::FilePath& filename)
{
    _captureFilename = filename;
    _captureRequested = true;
}

bool OslTestRenderPipeline::isFrameCaptureComplete() const
{
    return !_captureRequested;
}

bool OslTestRenderPipeline::isBusy() const
{
    return _busy;
}

bool OslTestRenderPipeline::isNodeDefSupported(const mx::NodeDefPtr& /*nodeDef*/)
{
    return true;
}

void OslTestRenderPipeline::setActive(bool active)
{
    const bool wasActive = _active.exchange(active);
    if (active && !wasActive)
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _resetRequested = true;
        }
        notifyWorker();
    }
    else if (!active && wasActive)
    {
        {
            std::lock_guard<std::mutex> lock(_sessionMutex);
            if (_session)
            {
                _session->cancel();
            }
        }
        notifyWorker();
    }
}

mx::ImageHandlerPtr OslTestRenderPipeline::getImageHandler() const
{
    return _imageHandler;
}

const mx::StringSet& OslTestRenderPipeline::getXincludeFiles() const
{
    return _xincludeFiles;
}

#endif // MATERIALX_BUILD_RENDER_OSL_TESTRENDER
