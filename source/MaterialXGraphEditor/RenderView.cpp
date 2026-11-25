//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGraphEditor/RenderView.h>

#include <MaterialXGraphEditor/GlslRenderPipeline.h>
#ifdef MATERIALX_BUILD_RENDER_CYCLES
#include <MaterialXGraphEditor/CyclesRenderPipeline.h>
#endif
#ifdef MATERIALX_BUILD_RENDER_OSL_TESTRENDER
#include <MaterialXGraphEditor/OslTestRenderPipeline.h>
#endif

#include <MaterialXRenderGlsl/External/Glad/glad.h>

#include <imgui.h>

#include <algorithm>
#include <iostream>

namespace
{

const mx::Vector3 VIEW_DEFAULT_CAMERA_POSITION(0.0f, 0.0f, 5.0f);
const mx::Vector3 VIEW_DEFAULT_CAMERA_TARGET(0.0f, 0.0f, 0.0f);
const mx::Vector3 VIEW_DEFAULT_CAMERA_UP(0.0f, 1.0f, 0.0f);
const float VIEW_DEFAULT_CAMERA_VIEW_ANGLE = 45.0f;
const float VIEW_DEFAULT_CAMERA_ZOOM = 1.0f;

void enableSRGBCallback(const ImDrawList*, const ImDrawCmd*)
{
    glEnable(GL_FRAMEBUFFER_SRGB);
}

void disableSRGBCallback(const ImDrawList*, const ImDrawCmd*)
{
    glDisable(GL_FRAMEBUFFER_SRGB);
}

} // anonymous namespace

RenderView::RenderView(mx::DocumentPtr doc,
                       mx::DocumentPtr stdLib,
                       const std::string& meshFilename,
                       const std::string& envRadianceFilename,
                       const mx::FileSearchPath& searchPath,
                       int viewWidth,
                       int viewHeight) :
    _document(doc),
    _stdLib(stdLib),
    _searchPath(searchPath),
    _meshFilename(meshFilename),
    _envRadianceFilename(envRadianceFilename),
    _viewCamera(mx::Camera::create()),
    _viewWidth(viewWidth),
    _viewHeight(viewHeight)
{
    // Resolve input filenames, taking both the provided search path and the
    // current working directory into account.
    mx::FileSearchPath localSearchPath = searchPath;
    localSearchPath.append(mx::FilePath::getCurrentPath());
    _meshFilename = localSearchPath.find(_meshFilename);
    _envRadianceFilename = localSearchPath.find(_envRadianceFilename);

    _cameraState.position = VIEW_DEFAULT_CAMERA_POSITION;
    _cameraState.target = VIEW_DEFAULT_CAMERA_TARGET;
    _cameraState.up = VIEW_DEFAULT_CAMERA_UP;
    _cameraState.viewAngle = VIEW_DEFAULT_CAMERA_VIEW_ANGLE;
    _cameraState.zoom = VIEW_DEFAULT_CAMERA_ZOOM;
}

std::vector<std::string> RenderView::getAvailableBackends()
{
    std::vector<std::string> backends = { "GLSL" };
#ifdef MATERIALX_BUILD_RENDER_CYCLES
    backends.push_back("Cycles");
#endif
#ifdef MATERIALX_BUILD_RENDER_OSL_TESTRENDER
    backends.push_back("OSL (testrender)");
#endif
    return backends;
}

RenderPipelinePtr RenderView::createPipeline(const std::string& backendName)
{
    if (backendName == "GLSL")
    {
        return std::make_shared<GlslRenderPipeline>(
            _document, _stdLib, _meshFilename.asString(),
            _envRadianceFilename.asString(), _searchPath,
            _viewWidth, _viewHeight);
    }
#ifdef MATERIALX_BUILD_RENDER_CYCLES
    if (backendName == "Cycles")
    {
        return std::make_shared<CyclesRenderPipeline>(
            _document, _stdLib, _meshFilename.asString(),
            _envRadianceFilename.asString(), _searchPath,
            _viewWidth, _viewHeight, _settings.renderPass);
    }
#endif
#ifdef MATERIALX_BUILD_RENDER_OSL_TESTRENDER
    if (backendName == "OSL (testrender)")
    {
        return std::make_shared<OslTestRenderPipeline>(
            _document, _stdLib, _meshFilename.asString(),
            _envRadianceFilename.asString(), _searchPath,
            _viewWidth, _viewHeight, _settings.renderPass);
    }
#endif
    return nullptr;
}

void RenderView::setRenderBackend(const std::string& backendName)
{
    if (_activePipeline && backendName == _activePipeline->getName())
    {
        return;
    }

    RenderPipelinePtr pipeline;
    auto pipelineIt = _pipelines.find(backendName);
    if (pipelineIt != _pipelines.end())
    {
        pipeline = pipelineIt->second;
    }
    else
    {
        pipeline = createPipeline(backendName);
        if (!pipeline)
        {
            std::cerr << "Unsupported render backend: " << backendName << std::endl;
            return;
        }

        pipeline->initialize();
        pipeline->resize(_viewWidth, _viewHeight);
        pipeline->setCamera(_viewCamera, _cameraState);
        pipeline->setSettings(_settings);
        _pipelines[backendName] = pipeline;
        _pipelineDocs[backendName] = _document;
    }

    // Deactivate the outgoing backend before switching, so asynchronous
    // backends (e.g. Cycles) pause their render threads while not displayed.
    if (_activePipeline && _activePipeline != pipeline)
    {
        _activePipeline->setActive(false);
    }

    _activePipeline = pipeline;

    // Re-synchronize a cached pipeline with the current document if a different
    // file was loaded while this backend was inactive.
    if (_pipelineDocs[backendName] != _document)
    {
        _activePipeline->setDocument(_document);
        _pipelineDocs[backendName] = _document;
    }

    applySettings(*_activePipeline);
    applyCamera(*_activePipeline);
    _activePipeline->resize(_viewWidth, _viewHeight);
    _activePipeline->setActive(true);
}

std::string RenderView::getBackendName() const
{
    return _activePipeline ? _activePipeline->getName() : mx::EMPTY_STRING;
}

RenderCapabilities RenderView::getCapabilities() const
{
    return _activePipeline ? _activePipeline->getCapabilities() : RenderCapabilities();
}

void RenderView::initialize()
{
    if (!_activePipeline)
    {
        setRenderBackend("GLSL");
    }
}

void RenderView::setDocument(mx::DocumentPtr document)
{
    _document = document;
    for (auto& entry : _pipelines)
    {
        entry.second->setDocument(_document);
        _pipelineDocs[entry.first] = _document;
    }
}

void RenderView::updateMaterials(mx::TypedElementPtr typedElem)
{
    if (_activePipeline)
    {
        _activePipeline->updateMaterials(typedElem);
    }
}

void RenderView::loadMesh(const mx::FilePath& filename)
{
    _meshFilename = filename;
    for (auto& entry : _pipelines)
    {
        entry.second->loadMesh(filename);
    }
}

void RenderView::drawContents()
{
    if (!_activePipeline)
    {
        return;
    }

    _activePipeline->resize(_viewWidth, _viewHeight);
    applyCamera(*_activePipeline);
    _activePipeline->render();
}

unsigned int RenderView::getRenderTextureId() const
{
    return _activePipeline ? _activePipeline->getColorTexture() : 0;
}

void RenderView::beginFrameDisplay()
{
    ImGui::GetWindowDrawList()->AddCallback(enableSRGBCallback, nullptr);
}

void RenderView::endFrameDisplay()
{
    ImGui::GetWindowDrawList()->AddCallback(disableSRGBCallback, nullptr);
}

void RenderView::applySettings(RenderPipeline& pipeline)
{
    pipeline.setSettings(_settings);
}

void RenderView::applyCamera(RenderPipeline& pipeline)
{
    pipeline.setCamera(_viewCamera, _cameraState);
}

mx::Vector3 RenderView::computeCameraEye() const
{
    mx::Matrix44 invArcball = _viewCamera->arcballMatrix().getInverse();
    mx::Vector3 offset = invArcball.transformVector(_cameraState.position - _cameraState.target);
    offset = offset * (1.0f / _cameraState.zoom);
    return _cameraState.target + offset;
}

mx::Vector3 RenderView::computeCameraUp() const
{
    mx::Matrix44 invArcball = _viewCamera->arcballMatrix().getInverse();
    return invArcball.transformVector(_cameraState.up);
}

void RenderView::setMouseButtonEvent(int button, bool down, mx::Vector2 pos)
{
    if ((button == 0) && !ImGui::IsKeyPressed(ImGuiKey_RightShift) && !ImGui::IsKeyPressed(ImGuiKey_LeftShift))
    {
        _viewCamera->arcballButtonEvent(pos, down);
    }
    else if ((button == 1) || ((button == 0) && ImGui::IsKeyDown(ImGuiKey_RightShift)) || ((button == 0) && ImGui::IsKeyDown(ImGuiKey_LeftShift)))
    {
        _userTranslationStart = _cameraState.userTranslation;
        _userTranslationActive = true;
        _userTranslationPixel = pos;
    }
    if ((button == 0) && !down)
    {
        _viewCamera->arcballButtonEvent(pos, false);
    }
    if (!down)
    {
        _userTranslationActive = false;
    }
    ++_cameraState.revision;
}

void RenderView::setMouseMotionEvent(mx::Vector2 pos)
{
    if (_viewCamera->applyArcballMotion(pos))
    {
        ++_cameraState.revision;
        return;
    }

    if (_userTranslationActive)
    {
        mx::Vector3 eye = computeCameraEye();
        mx::Vector3 lookAt = _cameraState.target;
        mx::Vector3 forward = (lookAt - eye).getNormalized();
        mx::Vector3 right = forward.cross(computeCameraUp()).getNormalized();
        mx::Vector3 camUp = right.cross(forward).getNormalized();

        float distance = (eye - lookAt).getMagnitude();
        const float pi = std::acos(-1.0f);
        float panScale = 2.0f * distance * std::tan(_cameraState.viewAngle * pi / 360.0f) /
                         std::max(1.0f, (float) _viewHeight);
        float dx = pos[0] - _userTranslationPixel[0];
        float dy = pos[1] - _userTranslationPixel[1];
        _cameraState.userTranslation = _userTranslationStart + (right * dx - camUp * dy) * panScale;
        ++_cameraState.revision;
    }
}

void RenderView::setScrollEvent(float scrollY)
{
    _cameraState.zoom = std::max(0.1f, _cameraState.zoom * ((scrollY > 0) ? 1.1f : 0.9f));
    ++_cameraState.revision;
}

void RenderView::setKeyEvent(int key)
{
    if (key == ImGuiKey_KeypadAdd)
    {
        _cameraState.zoom *= 1.1f;
        ++_cameraState.revision;
    }
    if (key == ImGuiKey_KeypadSubtract)
    {
        _cameraState.zoom = std::max(0.1f, _cameraState.zoom * 0.9f);
        ++_cameraState.revision;
    }
}

void RenderView::requestFrameCapture(const mx::FilePath& filename)
{
    if (_activePipeline)
    {
        _activePipeline->requestFrameCapture(filename);
    }
}

bool RenderView::isFrameCaptureComplete() const
{
    return !_activePipeline || _activePipeline->isFrameCaptureComplete();
}

void RenderView::requestExit()
{
    _exitRequested = true;
}

mx::ImageHandlerPtr RenderView::getImageHandler() const
{
    return _activePipeline ? _activePipeline->getImageHandler() : nullptr;
}

const mx::StringSet& RenderView::getXincludeFiles() const
{
    static const mx::StringSet empty;
    return _activePipeline ? _activePipeline->getXincludeFiles() : empty;
}

bool RenderView::isNodeDefSupported(const mx::NodeDefPtr& nodeDef)
{
    return _activePipeline && _activePipeline->isNodeDefSupported(nodeDef);
}

void RenderView::modifyUniform(const std::string& name, mx::ValuePtr value)
{
    if (_activePipeline)
    {
        _activePipeline->modifyUniform(name, value);
    }
}

void RenderView::setRenderPass(const std::string& name)
{
    _settings.renderPass = name;
    if (_activePipeline)
    {
        applySettings(*_activePipeline);
    }
}

void RenderView::setDenoise(bool enabled)
{
    _settings.denoise = enabled;
    if (_activePipeline)
    {
        applySettings(*_activePipeline);
    }
}

void RenderView::setDenoiseStartSample(int samples)
{
    _settings.denoiseStartSample = samples;
    if (_activePipeline)
    {
        applySettings(*_activePipeline);
    }
}

void RenderView::setDenoisePasses(int passes)
{
    _settings.denoisePasses = passes;
    if (_activePipeline)
    {
        applySettings(*_activePipeline);
    }
}

void RenderView::setAdaptiveSampling(bool enabled)
{
    _settings.adaptiveSampling = enabled;
    if (_activePipeline)
    {
        applySettings(*_activePipeline);
    }
}

void RenderView::setAdaptiveThreshold(float threshold)
{
    _settings.adaptiveThreshold = threshold;
    if (_activePipeline)
    {
        applySettings(*_activePipeline);
    }
}

void RenderView::setAdaptiveMinSamples(int samples)
{
    _settings.adaptiveMinSamples = samples;
    if (_activePipeline)
    {
        applySettings(*_activePipeline);
    }
}

void RenderView::setMaxSamples(int samples)
{
    _settings.maxSamples = samples;
    if (_activePipeline)
    {
        applySettings(*_activePipeline);
    }
}

void RenderView::setLightIntensity(float intensity)
{
    _settings.lightIntensity = intensity;
    if (_activePipeline)
    {
        applySettings(*_activePipeline);
    }
}

void RenderView::setActive(bool active)
{
    if (_activePipeline)
    {
        _activePipeline->setActive(active);
    }
}
