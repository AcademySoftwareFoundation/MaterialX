//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifdef MATERIALX_BUILD_RENDER_CYCLES

#include <MaterialXGraphEditor/CyclesRenderView.h>

#include <MaterialXRenderGlsl/GLTextureHandler.h>
#include <MaterialXRenderGlsl/External/Glad/glad.h>
#include <MaterialXRender/StbImageLoader.h>
#include <MaterialXRender/CgltfLoader.h>
#include <MaterialXRender/TinyObjLoader.h>
#include <MaterialXRender/Mesh.h>
#include <MaterialXRender/Util.h>

#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXFormat/Util.h>

#include <imgui.h>

#include "device/device.h"
#include "scene/attribute.h"
#include "scene/background.h"
#include "scene/camera.h"
#include "scene/integrator.h"
#include "scene/mesh.h"
#include "scene/object.h"
#include "scene/osl.h"
#include "scene/pass.h"
#include "scene/scene.h"
#include "scene/shader.h"
#include "scene/shader_graph.h"
#include "scene/shader_nodes.h"
#include "session/buffers.h"
#include "session/session.h"
#include "util/array.h"
#include "util/math.h"
#include "util/string.h"
#include "util/transform.h"
#include "util/types.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <mutex>
#include <vector>

//
// CyclesCaptureDisplayDriver
//
// A Cycles display driver that captures the rendered frame into a CPU buffer.
// Capturing to the CPU avoids sharing the OpenGL context with the Cycles
// render thread; the graph editor uploads the pixels to a texture on the main
// thread from drawContents().
//

class CyclesCaptureDisplayDriver : public ccl::DisplayDriver
{
  public:
    bool update_begin(const Params& /*params*/, const int texture_width, const int texture_height) override
    {
        _width = texture_width;
        _height = texture_height;
        if (texture_width > 0 && texture_height > 0)
        {
            _write.resize(static_cast<size_t>(texture_width) * texture_height);
        }
        return true;
    }

    void update_end() override
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _ready = std::move(_write);
        _readyWidth = _width;
        _readyHeight = _height;
        _hasFrame = true;
    }

    void next_tile_begin() override { }

    ccl::half4* map_texture_buffer() override
    {
        return _write.empty() ? nullptr : _write.data();
    }

    void unmap_texture_buffer() override { }

    void zero() override { }

    void draw(const Params& /*params*/) override { }

    std::vector<ccl::half4> takeFrame(int& width, int& height)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _hasFrame = false;
        width = _readyWidth;
        height = _readyHeight;
        return std::move(_ready);
    }

  private:
    std::mutex _mutex;
    std::vector<ccl::half4> _write;
    std::vector<ccl::half4> _ready;
    int _width = 0;
    int _height = 0;
    int _readyWidth = 0;
    int _readyHeight = 0;
    bool _hasFrame = false;
};

namespace
{

const double PI = std::acos(-1.0);

// Delay before a property change is applied, so that continuous edits (e.g.
// dragging a slider) only trigger a single OSL recompilation.
const double MATERIAL_SETTLE_SECONDS = 0.5;

double currentTimeSeconds()
{
    return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

// Cycles binds the OSL globals u,v to the barycentric triangle coordinates, so
// the MaterialX OSL texcoord implementation ("vector2(u,v)") must be remapped
// to the mesh UV attribute for Cycles.
const char* CYCLES_UV_HELPER =
    "vector2 mx_cycles_uv()\n"
    "{\n"
    "    point uv = point(0.0, 0.0, 0.0);\n"
    "    getattribute(\"geom:uv\", uv);\n"
    "    return vector2(uv.x, uv.y);\n"
    "}\n";

// Cycles' OSL runtime does not bind the OSL dPdu/dPdv globals, so the
// MaterialX-generated tangent ('normalize(transform(space, dPdu))') is
// invalid. Anisotropic specular/coat lobes then produce invalid shading
// (visible as colored speckles). Use the Cycles UV tangent attribute
// ('geom:tangent') when available, falling back to a stable tangent derived
// from the shading normal.
const char* CYCLES_TANGENT_HELPER =
    "vector mx_cycles_tangent(string space)\n"
    "{\n"
    "    vector tangent = vector(0.0, 0.0, 0.0);\n"
    "    getattribute(\"geom:tangent\", tangent);\n"
    "    tangent = transform(space, tangent);\n"
    "    vector n = normalize(transform(space, N));\n"
    "    if (length(tangent) < 1.0e-6)\n"
    "    {\n"
    "        vector up = (abs(n.y) < 0.99) ? vector(0.0, 1.0, 0.0) : vector(1.0, 0.0, 0.0);\n"
    "        tangent = cross(up, n);\n"
    "    }\n"
    "    return normalize(tangent);\n"
    "}\n";

std::string customizeOslForCycles(const std::string& source)
{
    std::string result = source;

    size_t insertPos = 0;
    size_t searchFrom = 0;
    size_t includePos = 0;
    while ((includePos = result.find("#include", searchFrom)) != std::string::npos)
    {
        size_t eol = result.find('\n', includePos);
        if (eol == std::string::npos)
        {
            insertPos = result.size();
            break;
        }
        insertPos = eol + 1;
        searchFrom = eol + 1;
    }
    result.insert(insertPos, std::string("\n") + CYCLES_UV_HELPER + "\n" +
                                  CYCLES_TANGENT_HELPER + "\n");

    auto replaceAll = [](std::string& text, const std::string& from, const std::string& to)
    {
        size_t pos = 0;
        while ((pos = text.find(from, pos)) != std::string::npos)
        {
            text.replace(pos, from.size(), to);
            pos += to.size();
        }
    };

    replaceAll(result, "vector2(u,v)", "mx_cycles_uv()");
    replaceAll(result, "vector(u,v,0)", "vector(mx_cycles_uv(), 0)");

    // Replace the MaterialX tangent (which relies on the unbound OSL dPdu).
    replaceAll(result, "normalize(transform(geomprop_Tworld_space, dPdu))",
               "mx_cycles_tangent(geomprop_Tworld_space)");

    // Cycles' OSL shading system does not implement the OSL standard library's
    // anisotropic_vdf closure, which the generated open_pbr_surface uses for
    // its transmission volume. An unsupported closure evaluates to empty and
    // turns the whole material black. The transmission BSDF already provides
    // the surface refraction for the preview, so map the VDF to an empty
    // closure. This also avoids a division-by-zero in the albedo when
    // transmission_depth is zero (extinction = absorption + scattering = 0).
    // Worth reporting upstream: Cycles should register the standard-library
    // VDF closures, or the MaterialX open_pbr_surface OSL implementation
    // should not rely on them.
    replaceAll(result, "anisotropic_vdf(albedo, extinction, anisotropy)",
               "null_closure()");

    return result;
}

ccl::float3 toFloat3(const mx::Vector3& v)
{
    return ccl::make_float3(v[0], v[1], v[2]);
}

ccl::float3 toFloat3(const mx::Color3& c)
{
    return ccl::make_float3(c[0], c[1], c[2]);
}

// ImGui draw-list callbacks that toggle sRGB encoding while the render view is
// drawn, mirroring GlslRenderView::beginFrameDisplay. The Cycles display
// texture holds linear scene-referred data (GL_RGBA32F), so without this the
// preview is written to the default framebuffer without the linear-to-sRGB
// encode and appears far darker than the captured images.
void enableSRGBCallback(const ImDrawList*, const ImDrawCmd*)
{
    glEnable(GL_FRAMEBUFFER_SRGB);
}

void disableSRGBCallback(const ImDrawList*, const ImDrawCmd*)
{
    glDisable(GL_FRAMEBUFFER_SRGB);
}

ccl::PassType passTypeFromName(const std::string& name)
{
    if (name == "albedo" || name == "diffuse_color")
    {
        return ccl::PASS_DIFFUSE_COLOR;
    }
    return ccl::PASS_COMBINED;
}

} // anonymous namespace

//
// CyclesRenderView methods
//

void CyclesRenderView::beginFrameDisplay()
{
    ImGui::GetWindowDrawList()->AddCallback(enableSRGBCallback, nullptr);
}

void CyclesRenderView::endFrameDisplay()
{
    ImGui::GetWindowDrawList()->AddCallback(disableSRGBCallback, nullptr);
}

CyclesRenderView::CyclesRenderView(mx::DocumentPtr doc,
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
    _meshFilename(meshFilename),
    _envRadianceFilename(envRadianceFilename),
    _genContext(std::make_unique<mx::GenContext>(mx::OslShaderGenerator::create())),
    _renderPass(renderPass),
    _cameraPosition(0.0f, 0.0f, 5.0f),
    _cameraTarget(0.0f, 0.0f, 0.0f),
    _cameraUp(0.0f, 1.0f, 0.0f),
    _cameraFov(45.0f)
{
    _viewWidth = viewWidth;
    _viewHeight = viewHeight;

    // Resolve input filenames, taking both the provided search path and the
    // current working directory into account.
    mx::FileSearchPath localSearchPath = _searchPath;
    localSearchPath.append(mx::FilePath::getCurrentPath());
    _meshFilename = localSearchPath.find(_meshFilename);
    _envRadianceFilename = localSearchPath.find(_envRadianceFilename);

    _viewCamera = mx::Camera::create();

    setDocument(doc);
}

CyclesRenderView::~CyclesRenderView()
{
    if (_session)
    {
        _session->cancel();
        _session.reset();
    }
    if (_imageHandler && _image)
    {
        _imageHandler->releaseRenderResources(_image);
    }
}

void CyclesRenderView::initialize()
{
    _imageHandler = mx::GLTextureHandler::create(mx::StbImageLoader::create());
    _imageHandler->setSearchPath(_searchPath);

    buildScene();
}

void CyclesRenderView::buildScene()
{
#ifdef CYCLES_SHADER_DIR
    // Cycles locates its OSL node shaders and stdcycles.h through
    // path_get("shader"), which is derived from the host executable's path.
    // Point it at the Cycles installation so that OSL shading works when
    // embedded here.
    if (getenv("CYCLES_SHADER_PATH") == nullptr)
    {
        setenv("CYCLES_SHADER_PATH", CYCLES_SHADER_DIR, 0);
    }
#endif

    const auto devices = ccl::Device::available_devices(ccl::DEVICE_MASK_CPU);
    if (devices.empty())
    {
        std::cerr << "Cycles: no CPU device available" << std::endl;
        return;
    }

    // The CPU denoiser is OpenImageDenoise. Disable denoising when this Cycles
    // build was compiled without it.
    _denoiseSupported = (devices.front().denoisers & ccl::DENOISER_OPENIMAGEDENOISE) != 0;

    ccl::SessionParams sessionParams;
    sessionParams.device = devices.front();
    sessionParams.background = false;
    sessionParams.headless = false;
    sessionParams.samples = _maxSamples;
    sessionParams.use_auto_tile = false;
    sessionParams.use_resolution_divider = false;
    sessionParams.shadingsystem = ccl::SHADINGSYSTEM_OSL;

    ccl::SceneParams sceneParams;
    sceneParams.shadingsystem = ccl::SHADINGSYSTEM_OSL;

    _session = std::make_unique<ccl::Session>(sessionParams, sceneParams);

    ccl::Scene* scene = _session->scene.get();

    // Custom OSL shaders access the mesh UVs through getattribute("geom:uv").
    // Adding a UV pass marks the UV attribute as globally required so that it
    // is exported for the geometry.
    scene->create_node<ccl::Pass>()->set_type(ccl::PASS_UV);

    // Select the pass shown in the viewport (and written to captures).
    applyDisplayPass(scene);

    // Configure denoising and adaptive sampling before any samples are rendered.
    applyRenderSettings(scene);

    // Build the mesh used by the GLSL render view when available, otherwise
    // fall back to a simple sphere so that there is always something to render.
    if (!buildMesh(scene))
    {
        buildFallbackSphere(scene);
    }

    // Give the default surface a visible color.
    {
        auto graph = std::make_unique<ccl::ShaderGraph>();
        ccl::PrincipledBsdfNode* bsdf = graph->create_node<ccl::PrincipledBsdfNode>();
        bsdf->set_base_color(ccl::make_float3(0.8f, 0.5f, 0.2f));
        bsdf->set_roughness(0.3f);
        graph->connect(bsdf->output("BSDF"), graph->output()->input("Surface"));
        scene->default_surface->set_graph(std::move(graph));
        scene->default_surface->tag_update(scene);
    }

    buildEnvironment(scene);

    // Camera setup.
    scene->camera->set_camera_type(ccl::CAMERA_PERSPECTIVE);
    scene->camera->set_full_width(_viewWidth);
    scene->camera->set_full_height(_viewHeight);
    scene->camera->set_fov(_cameraFov * (float) PI / 180.0f);
    scene->camera->compute_auto_viewplane();
    updateCamera();

    // Capture display driver.
    auto displayDriver = std::make_unique<CyclesCaptureDisplayDriver>();
    _displayDriver = displayDriver.get();
    _session->set_display_driver(std::move(displayDriver));

    ccl::BufferParams bufferParams;
    bufferParams.width = _viewWidth;
    bufferParams.height = _viewHeight;
    bufferParams.full_width = _viewWidth;
    bufferParams.full_height = _viewHeight;

    _session->reset(sessionParams, bufferParams);
    _sessionWidth = _viewWidth;
    _sessionHeight = _viewHeight;

    // Compile and apply the initial material before the session starts
    // rendering. Otherwise the placeholder surface above (used only to keep the
    // scene non-empty) would be displayed for the first frames, which the user
    // sees as a flash of a gold sphere when first switching to this backend.
    generateOsl(_currentElement);
    rebuildMaterial();
    _materialDirty = false;
    _materialImmediate = false;

    _session->start();
    _active = true;
}

bool CyclesRenderView::buildMesh(ccl::Scene* scene)
{
    if (_meshFilename.isEmpty() || !_meshFilename.exists())
    {
        return false;
    }

    _geometryHandler = mx::GeometryHandler::create();
    _geometryHandler->addLoader(mx::TinyObjLoader::create());
    _geometryHandler->addLoader(mx::CgltfLoader::create());
    if (!_geometryHandler->loadGeometry(_meshFilename) || _geometryHandler->getMeshes().empty())
    {
        _geometryHandler = nullptr;
        return false;
    }

    // Normalize the mesh to the same bounding sphere that the GLSL render view
    // uses, so that the two previews line up in scale and position.
    const mx::Vector3 boxMax = _geometryHandler->getMaximumBounds();
    const mx::Vector3 boxMin = _geometryHandler->getMinimumBounds();
    const mx::Vector3 sphereCenter = (boxMax + boxMin) * 0.5f;
    const float radius = (sphereCenter - boxMin).getMagnitude();
    const float scale = (radius > 0.0f) ? (2.0f / radius) : 1.0f;

    std::vector<ccl::packed_float3> positions;
    std::vector<ccl::packed_float3> normals;
    std::vector<ccl::float2> texcoords;
    std::vector<int> triangles;

    for (const mx::MeshPtr& mesh : _geometryHandler->getMeshes())
    {
        mx::MeshStreamPtr positionStream = mesh->getStream(mx::MeshStream::POSITION_ATTRIBUTE, 0);
        if (!positionStream)
        {
            continue;
        }

        mx::MeshStreamPtr normalStream = mesh->getStream(mx::MeshStream::NORMAL_ATTRIBUTE, 0);
        mx::MeshStreamPtr texcoordStream = mesh->getStream(mx::MeshStream::TEXCOORD_ATTRIBUTE, 0);

        const size_t vertexOffset = positions.size();
        const mx::MeshFloatBuffer& positionData = positionStream->getData();
        const size_t vertexCount = positionStream->getSize();
        for (size_t i = 0; i < vertexCount; i++)
        {
            positions.emplace_back(ccl::make_float3((positionData[i * 3 + 0] - sphereCenter[0]) * scale,
                                                    (positionData[i * 3 + 1] - sphereCenter[1]) * scale,
                                                    (positionData[i * 3 + 2] - sphereCenter[2]) * scale));
        }

        if (normalStream)
        {
            const mx::MeshFloatBuffer& normalData = normalStream->getData();
            const size_t normalCount = normalStream->getSize();
            for (size_t i = 0; i < normalCount; i++)
            {
                normals.emplace_back(ccl::make_float3(normalData[i * 3 + 0],
                                                      normalData[i * 3 + 1],
                                                      normalData[i * 3 + 2]));
            }
        }

        if (texcoordStream)
        {
            const mx::MeshFloatBuffer& texcoordData = texcoordStream->getData();
            const size_t texcoordCount = texcoordStream->getSize();
            for (size_t i = 0; i < texcoordCount; i++)
            {
                texcoords.emplace_back(ccl::make_float2(texcoordData[i * 2 + 0],
                                                        texcoordData[i * 2 + 1]));
            }
        }

        for (size_t partIndex = 0; partIndex < mesh->getPartitionCount(); partIndex++)
        {
            mx::MeshPartitionPtr part = mesh->getPartition(partIndex);
            for (uint32_t index : part->getIndices())
            {
                triangles.push_back((int) (index + vertexOffset));
            }
        }
    }

    if (positions.empty() || triangles.empty())
    {
        _geometryHandler = nullptr;
        return false;
    }

    const size_t triangleCount = triangles.size() / 3;

    ccl::Mesh* cyclesMesh = scene->create_node<ccl::Mesh>();
    cyclesMesh->resize_mesh((int) positions.size(), (int) triangleCount);

    ccl::float3* cyclePositions = cyclesMesh->get_verts().data();
    for (size_t i = 0; i < positions.size(); i++)
    {
        cyclePositions[i] = positions[i];
    }

    int* cycleTriangles = cyclesMesh->get_triangles().data();
    for (size_t i = 0; i < triangles.size(); i++)
    {
        cycleTriangles[i] = triangles[i];
    }

    std::ranges::fill(cyclesMesh->get_smooth(), true);
    std::ranges::fill(cyclesMesh->get_shader(), 0);

    if (!normals.empty() && normals.size() == positions.size())
    {
        if (ccl::Attribute* attribute = cyclesMesh->attributes.add(ccl::ATTR_STD_VERTEX_NORMAL))
        {
            ccl::packed_normal* normalData = attribute->data_normal();
            for (size_t i = 0; i < normals.size(); i++)
            {
                normalData[i] = ccl::packed_normal(normals[i]);
            }
        }
    }

    if (!texcoords.empty())
    {
        if (ccl::Attribute* attribute = cyclesMesh->attributes.add(ccl::ATTR_STD_UV))
        {
            ccl::float2* uvData = attribute->data_float2();
            for (size_t t = 0; t < triangleCount; t++)
            {
                for (int k = 0; k < 3; k++)
                {
                    const int vertexIndex = triangles[t * 3 + k];
                    if (vertexIndex < (int) texcoords.size())
                    {
                        uvData[t * 3 + k] = texcoords[vertexIndex];
                    }
                }
            }
        }
    }

    cyclesMesh->tag_triangles_modified();
    cyclesMesh->tag_shader_modified();
    cyclesMesh->tag_smooth_modified();

    if (normals.empty())
    {
        cyclesMesh->add_vertex_normals();
    }

    ccl::Object* object = scene->create_node<ccl::Object>();
    object->set_geometry(cyclesMesh);
    object->set_tfm(ccl::transform_identity());
    _object = object;

    return true;
}

void CyclesRenderView::buildFallbackSphere(ccl::Scene* scene)
{
    ccl::Mesh* mesh = scene->create_node<ccl::Mesh>();

    const int segments = 64;
    const int rings = 32;
    const float radius = 1.0f;

    std::vector<ccl::packed_float3> positions;
    positions.reserve((rings + 1) * (segments + 1));
    for (int y = 0; y <= rings; y++)
    {
        const float v = (float) y / (float) rings;
        const float theta = v * (float) PI;
        for (int x = 0; x <= segments; x++)
        {
            const float u = (float) x / (float) segments;
            const float phi = u * 2.0f * (float) PI;
            positions.emplace_back(ccl::make_float3(radius * std::sin(theta) * std::cos(phi),
                                                    radius * std::cos(theta),
                                                    radius * std::sin(theta) * std::sin(phi)));
        }
    }

    std::vector<int> indices;
    indices.reserve(rings * segments * 6);
    const int rowStride = segments + 1;
    for (int y = 0; y < rings; y++)
    {
        for (int x = 0; x < segments; x++)
        {
            const int a = y * rowStride + x;
            const int b = a + rowStride;
            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(a + 1);
            indices.push_back(a + 1);
            indices.push_back(b);
            indices.push_back(b + 1);
        }
    }

    mesh->resize_mesh((int) positions.size(), (int) (indices.size() / 3));
    std::copy(positions.begin(), positions.end(), mesh->get_verts().data());
    int* triangles = mesh->get_triangles().data();
    std::copy(indices.begin(), indices.end(), triangles);
    std::ranges::fill(mesh->get_smooth(), true);
    std::ranges::fill(mesh->get_shader(), 0);
    mesh->tag_triangles_modified();
    mesh->tag_smooth_modified();
    mesh->tag_shader_modified();

    ccl::Object* object = scene->create_node<ccl::Object>();
    object->set_geometry(mesh);
    object->set_tfm(ccl::transform_identity());
    _object = object;
}

bool CyclesRenderView::buildEnvironment(ccl::Scene* scene)
{
    // The GLSL render view clears the viewport to a constant screen color and
    // uses the environment only for lighting. Replicate that: the background
    // visible to camera rays is the screen color, while shadow and indirect
    // rays see the environment.
    const ccl::float3 screenColor = toFloat3(mx::DEFAULT_SCREEN_COLOR_LIN_REC709);

    _environmentBackground = nullptr;
    _irradianceBackground = nullptr;

    // MaterialX lat-long environments are Y-up, while Cycles' equirectangular
    // projection is Z-up. Rotate the environment lookup so that the scene's +Y
    // up axis maps to the environment zenith, matching the GLSL render view.
    const ccl::float3 environmentRotation = ccl::make_float3(-(float) PI / 2.0f, 0.0f, 0.0f);

    auto graph = std::make_unique<ccl::ShaderGraph>();
    ccl::LightPathNode* lightPath = graph->create_node<ccl::LightPathNode>();

    // Camera rays see the screen color; indirect rays see the environment.
    ccl::MixClosureNode* cameraMix = graph->create_node<ccl::MixClosureNode>();
    graph->connect(lightPath->output("Is Camera Ray"), cameraMix->input("Fac"));

    ccl::BackgroundNode* screen = graph->create_node<ccl::BackgroundNode>();
    screen->set_color(screenColor);
    screen->set_strength(1.0f);
    graph->connect(screen->output("Background"), cameraMix->input("Closure2"));

    bool hasEnvironment = !_envRadianceFilename.isEmpty() && _envRadianceFilename.exists();
    if (hasEnvironment)
    {
        // Radiance map: sharp, used for specular/transmission rays so that
        // reflective materials show the HDRI, matching the GLSL backend which
        // reflects the radiance map for specular.
        ccl::EnvironmentTextureNode* radiance = graph->create_node<ccl::EnvironmentTextureNode>();
        radiance->set_filename(ccl::ustring(_envRadianceFilename.asString()));
        radiance->set_tex_mapping_rotation(environmentRotation);
        ccl::BackgroundNode* radianceBackground = graph->create_node<ccl::BackgroundNode>();
        _environmentBackground = radianceBackground;
        _baseEnvironmentStrength = 3.0f;
        radianceBackground->set_strength(_baseEnvironmentStrength * _lightIntensity);
        graph->connect(radiance->output("Color"), radianceBackground->input("Color"));

        // Irradiance map: a cosine-weighted average, used for diffuse rays as
        // the GLSL backend does for ambient diffuse. Its strength is boosted
        // empirically because a prefiltered irradiance map under-lights when
        // re-integrated over the hemisphere.
        mx::FilePath irradianceFilename = _envRadianceFilename.getParentPath() /
                                          "irradiance" / _envRadianceFilename.getBaseName();
        if (irradianceFilename.exists())
        {
            ccl::EnvironmentTextureNode* irradiance = graph->create_node<ccl::EnvironmentTextureNode>();
            irradiance->set_filename(ccl::ustring(irradianceFilename.asString()));
            irradiance->set_tex_mapping_rotation(environmentRotation);
            ccl::BackgroundNode* irradianceBackground = graph->create_node<ccl::BackgroundNode>();
            _irradianceBackground = irradianceBackground;
            _baseIrradianceStrength = 6.0f;
            irradianceBackground->set_strength(_baseIrradianceStrength * _lightIntensity);
            graph->connect(irradiance->output("Color"), irradianceBackground->input("Color"));

            // Diffuse rays see the irradiance map; all other indirect rays
            // (glossy, reflection, transmission) see the sharp radiance map.
            ccl::MixClosureNode* environmentMix = graph->create_node<ccl::MixClosureNode>();
            graph->connect(lightPath->output("Is Diffuse Ray"), environmentMix->input("Fac"));
            graph->connect(radianceBackground->output("Background"), environmentMix->input("Closure1"));
            graph->connect(irradianceBackground->output("Background"), environmentMix->input("Closure2"));
            graph->connect(environmentMix->output("Closure"), cameraMix->input("Closure1"));
        }
        else
        {
            graph->connect(radianceBackground->output("Background"), cameraMix->input("Closure1"));
        }
    }
    else
    {
        ccl::BackgroundNode* ambient = graph->create_node<ccl::BackgroundNode>();
        ambient->set_color(ccl::make_float3(0.5f, 0.5f, 0.5f));
        ambient->set_strength(1.0f);
        _environmentBackground = ambient;
        _baseEnvironmentStrength = 1.0f;
        ambient->set_strength(_baseEnvironmentStrength * _lightIntensity);
        graph->connect(ambient->output("Background"), cameraMix->input("Closure1"));
    }

    graph->connect(cameraMix->output("Closure"), graph->output()->input("Surface"));
    scene->default_background->set_graph(std::move(graph));
    scene->default_background->tag_update(scene);

    return hasEnvironment;
}

void CyclesRenderView::applyLightIntensity()
{
    if (!_session)
    {
        return;
    }

    ccl::Scene* scene = _session->scene.get();
    if (_environmentBackground)
    {
        _environmentBackground->set_strength(_baseEnvironmentStrength * _lightIntensity);
        scene->default_background->tag_update(scene);
    }
    if (_irradianceBackground)
    {
        _irradianceBackground->set_strength(_baseIrradianceStrength * _lightIntensity);
        scene->default_background->tag_update(scene);
    }
    restartRender();
}

void CyclesRenderView::setLightIntensity(float intensity)
{
    if (intensity == _lightIntensity)
    {
        return;
    }
    _lightIntensity = intensity;
    applyLightIntensity();
}

void CyclesRenderView::applyDisplayPass(ccl::Scene* scene)
{
    // The UV pass is always present so that custom OSL shaders can read
    // getattribute("geom:uv"); the display pass selects which result is shown.
    scene->film->set_display_pass(passTypeFromName(_renderPass));
    scene->film->tag_modified();
}

void CyclesRenderView::applyRenderSettings(ccl::Scene* scene)
{
    ccl::Integrator* integrator = scene->integrator;

    // Denoising only applies to the combined (beauty) image. Applying it while
    // a data pass such as albedo is displayed produces an empty buffer, so
    // scope it to the combined pass.
    integrator->set_use_denoise(isDenoiseActive());
    // Use OpenImageDenoise on the CPU so that denoising works on any machine,
    // independent of GPU vendor support.
    integrator->set_denoiser_type(ccl::DENOISER_OPENIMAGEDENOISE);
    integrator->set_denoise_use_gpu(false);
    integrator->set_denoise_start_sample(_denoiseStartSample);

    integrator->set_use_adaptive_sampling(_adaptiveSampling);
    integrator->set_adaptive_threshold(_adaptiveThreshold);
    integrator->set_adaptive_min_samples(_adaptiveMinSamples);

    // Clamp direct-light samples to suppress fireflies from the sharp sun on
    // glossy/anisotropic OSL materials (the GLSL backend is not path traced and
    // has no equivalent noise).
    integrator->set_sample_clamp_direct(10.0f);
    integrator->set_filter_glossy(4.0f);
    integrator->set_sample_clamp_indirect(10.0f);

    integrator->tag_update(scene, ccl::Integrator::UPDATE_ALL);
}

bool CyclesRenderView::isDenoiseActive() const
{
    return _denoise && _denoiseSupported && _renderPass == "combined";
}

void CyclesRenderView::setDenoise(bool enabled)
{
    if (enabled == _denoise)
    {
        return;
    }
    _denoise = enabled;
    if (_session)
    {
        applyRenderSettings(_session->scene.get());
        restartRender();
    }
}

void CyclesRenderView::setDenoiseStartSample(int samples)
{
    if (samples == _denoiseStartSample)
    {
        return;
    }
    _denoiseStartSample = samples;
    if (_session)
    {
        applyRenderSettings(_session->scene.get());
        restartRender();
    }
}

void CyclesRenderView::setAdaptiveSampling(bool enabled)
{
    if (enabled == _adaptiveSampling)
    {
        return;
    }
    _adaptiveSampling = enabled;
    if (_session)
    {
        applyRenderSettings(_session->scene.get());
        restartRender();
    }
}

void CyclesRenderView::setAdaptiveThreshold(float threshold)
{
    if (threshold == _adaptiveThreshold)
    {
        return;
    }
    _adaptiveThreshold = threshold;
    if (_session)
    {
        applyRenderSettings(_session->scene.get());
        restartRender();
    }
}

void CyclesRenderView::setAdaptiveMinSamples(int samples)
{
    if (samples == _adaptiveMinSamples)
    {
        return;
    }
    _adaptiveMinSamples = samples;
    if (_session)
    {
        applyRenderSettings(_session->scene.get());
        restartRender();
    }
}

void CyclesRenderView::setMaxSamples(int samples)
{
    if (samples == _maxSamples)
    {
        return;
    }
    _maxSamples = samples;
    if (_session)
    {
        _session->set_samples(samples);
        restartRender();
    }
}

void CyclesRenderView::setRenderPass(const std::string& name)
{
    if (name == _renderPass)
    {
        return;
    }
    _renderPass = name;

    if (_session)
    {
        applyDisplayPass(_session->scene.get());
        // Denoising is scoped to the combined pass, so re-apply the settings.
        applyRenderSettings(_session->scene.get());
        restartRender();
    }
}

void CyclesRenderView::setActive(bool active)
{
    if (active == _active)
    {
        return;
    }
    _active = active;

    if (!_session)
    {
        return;
    }

    // Pause the render thread while this backend is not displayed so it does
    // not consume CPU in the background behind another backend.
    _session->set_pause(!active);

    if (active)
    {
        // Restart so the first displayed frame reflects the current document
        // and settings rather than samples accumulated before the switch.
        restartRender();
    }
}

void CyclesRenderView::updateCamera()
{
    if (!_session)
    {
        return;
    }

    ccl::Scene* scene = _session->scene.get();
    ccl::Camera* camera = scene->camera;

    camera->set_full_width(_viewWidth);
    camera->set_full_height(_viewHeight);
    camera->set_fov(_cameraFov * (float) PI / 180.0f);
    camera->compute_auto_viewplane();

    const ccl::float3 eye = toFloat3(computeCameraEye());
    const ccl::float3 target = toFloat3(_cameraTarget);
    const ccl::float3 upHint = toFloat3(computeCameraUp());

    const ccl::float3 forward = ccl::normalize(target - eye);
    const ccl::float3 right = ccl::normalize(ccl::cross(forward, upHint));
    const ccl::float3 up = ccl::cross(right, forward);

    // Cycles stores the transform as rows, and applies it as a column-vector
    // matrix, so the camera basis vectors go into the columns of the matrix.
    ccl::Transform matrix = ccl::transform_identity();
    matrix.x = ccl::make_float4(right.x, up.x, forward.x, eye.x);
    matrix.y = ccl::make_float4(right.y, up.y, forward.y, eye.y);
    matrix.z = ccl::make_float4(right.z, up.z, forward.z, eye.z);

    camera->set_matrix(matrix);
    camera->need_flags_update = true;
    camera->need_device_update = true;

    // Panning moves the model rather than the camera, so that orbiting keeps
    // pivoting around the model center (matching the GLSL render view).
    if (_object)
    {
        _object->set_tfm(ccl::transform_translate(toFloat3(_userTranslation)));
        _object->tag_update(scene);
    }
}

void CyclesRenderView::restartRender()
{
    if (!_session)
    {
        return;
    }

    updateCamera();

    ccl::BufferParams bufferParams;
    bufferParams.width = _viewWidth;
    bufferParams.height = _viewHeight;
    bufferParams.full_width = _viewWidth;
    bufferParams.full_height = _viewHeight;

    _session->reset(_session->params, bufferParams);
    _sessionWidth = _viewWidth;
    _sessionHeight = _viewHeight;
}

void CyclesRenderView::uploadFrame(const std::vector<ccl::half4>& pixels, int width, int height)
{
    if (!_imageHandler || width <= 0 || height <= 0)
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
    const size_t count = (size_t) width * (size_t) height;
    for (size_t i = 0; i < count; i++)
    {
        const ccl::half4& pixel = pixels[i];
        float rgba[4] = { ccl::half_to_float(pixel.x),
                          ccl::half_to_float(pixel.y),
                          ccl::half_to_float(pixel.z),
                          ccl::half_to_float(pixel.w) };
        for (int c = 0; c < 4; c++)
        {
            // OSL closures can occasionally produce non-finite values;
            // sanitize them so they do not become colored speckles.
            if (!std::isfinite(rgba[c]))
            {
                rgba[c] = 0.0f;
            }
            destination[i * 4 + c] = rgba[c];
        }
    }

    if (_imageHandler->createRenderResources(_image, false))
    {
        _textureID = _image->getResourceId();

        // GLTextureHandler::createRenderResources does not set texture
        // filtering, and the default minification filter requires mipmaps,
        // which would make the texture incomplete and sample as black.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void CyclesRenderView::drawContents()
{
    if (!_session || !_displayDriver)
    {
        return;
    }

    // Clear the interaction flag if the release event was missed (e.g. the
    // button was released outside the viewport).
    if (_cameraInteracting && !ImGui::IsMouseDown(0) && !ImGui::IsMouseDown(1))
    {
        _cameraInteracting = false;
        _userTranslationActive = false;
        _viewCamera->arcballButtonEvent(mx::Vector2(), false);
    }

    // Rebuild the surface shader if the selected material or a property
    // changed. Structural changes are applied immediately; property edits are
    // debounced so that continuous changes cause a single recompilation.
    if (_materialDirty)
    {
        const bool ready = _materialImmediate ||
                           (currentTimeSeconds() - _materialDirtyTime >= MATERIAL_SETTLE_SECONDS);
        if (ready)
        {
            _materialDirty = false;
            _materialImmediate = false;
            generateOsl(_currentElement);
            rebuildMaterial();
        }
    }

    // Restart the progressive render when the camera or view size changed.
    if (_viewWidth != _sessionWidth || _viewHeight != _sessionHeight)
    {
        restartRender();
    }
    if (_cameraDirty)
    {
        _cameraDirty = false;
        restartRender();
    }

    int width = 0;
    int height = 0;
    std::vector<ccl::half4> frame = _displayDriver->takeFrame(width, height);
    if (!frame.empty() && width > 0 && height > 0)
    {
        uploadFrame(frame, width, height);
    }

    if (_captureRequested && _image)
    {
        // Cycles renders asynchronously: wait until the progressive render has
        // accumulated samples before writing the captured frame. When denoising
        // is enabled, also wait for a denoised result so that captures are not
        // written from a noisy frame. `get_current_sample()` reports the samples
        // rendered in the single render tile, while `get_denoised_tiles()`
        // becoming non-zero covers adaptive sampling that converges and
        // denoises before the configured start sample is reached.
        bool denoiseReady = true;
        if (isDenoiseActive() && _session)
        {
            denoiseReady = _session->progress.get_current_sample() >= _denoiseStartSample ||
                           _session->progress.get_denoised_tiles() > 0;
        }

        const double now = std::chrono::duration<double>(
                               std::chrono::steady_clock::now().time_since_epoch())
                               .count();
        if (denoiseReady && now - _captureRequestTime >= 2.0)
        {
            mx::ImagePtr saveImage = _image;
            if (_image->getBaseType() != mx::Image::BaseType::UINT8)
            {
                // Convert the HDR float buffer to 8-bit for saving, clamping to
                // [0, 1]. Image::setTexelColor does not clamp and casting an
                // out-of-range float to uint8 wraps around, which turned bright
                // highlights into colored (green) splotches in the saved PNG.
                // The interactive preview samples the float texture directly and
                // is clamped by the GPU, so it did not show the artifact.
                //
                // The buffer is scene-linear, while 8-bit formats such as PNG
                // are display-referred: encode the RGB channels to sRGB so the
                // capture matches the on-screen preview and the GLSL backend,
                // whose readback returns sRGB-encoded bytes. Alpha is linear
                // and is left untouched.
                saveImage = mx::Image::create(_image->getWidth(), _image->getHeight(), 4,
                                              mx::Image::BaseType::UINT8);
                saveImage->createResourceBuffer();
                for (unsigned int y = 0; y < _image->getHeight(); y++)
                {
                    for (unsigned int x = 0; x < _image->getWidth(); x++)
                    {
                        mx::Color4 color = _image->getTexelColor(x, y);
                        for (int c = 0; c < 4; c++)
                        {
                            color[c] = std::isfinite(color[c]) ? std::clamp(color[c], 0.0f, 1.0f) : 0.0f;
                        }
                        const mx::Color3 srgb = mx::Color3(color[0], color[1], color[2]).linearToSrgb();
                        color[0] = srgb[0];
                        color[1] = srgb[1];
                        color[2] = srgb[2];
                        saveImage->setTexelColor(x, y, color);
                    }
                }
            }
            if (_imageHandler->saveImage(_captureFilename, saveImage, true))
            {
                _captureRequested = false;
                std::cout << "Wrote frame to disk: " << _captureFilename.asString() << std::endl;
            }
            else
            {
                _captureRequested = false;
                std::cerr << "Failed to write frame to disk: " << _captureFilename.asString() << std::endl;
            }
        }
    }

    if (_exitRequested)
    {
        _exitRequested = false;
    }
}

void CyclesRenderView::setDocument(mx::DocumentPtr document)
{
    _document = document;
    _currentElement = nullptr;
    initContext(*_genContext);
    _materialDirty = true;
    _materialImmediate = true;
    _materialDirtyTime = currentTimeSeconds();
}

void CyclesRenderView::updateMaterials(mx::TypedElementPtr typedElem)
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

    // Structural changes (material selection, graph edits) are applied on the
    // next frame; only property/uniform edits are debounced.
    _materialDirty = true;
    _materialImmediate = true;
    _materialDirtyTime = currentTimeSeconds();
}

void CyclesRenderView::initContext(mx::GenContext& context)
{
    if (!_document)
    {
        return;
    }

    // Initialize search paths, including the folder holding the MaterialX OSL
    // support headers (e.g. mx_funcs.h).
    context.registerSourceCodeSearchPath(_searchPath);
    mx::FilePath genOslIncludePath = _searchPath.find("libraries/stdlib/genosl/include");
    if (!genOslIncludePath.isEmpty())
    {
        context.registerSourceCodeSearchPath(genOslIncludePath);
    }

    // Initialize unit management.
    mx::UnitTypeDefPtr distanceTypeDef = _document->getUnitTypeDef("distance");
    mx::LinearUnitConverterPtr distanceConverter = mx::LinearUnitConverter::create(distanceTypeDef);
    mx::UnitConverterRegistryPtr unitRegistry = mx::UnitConverterRegistry::create();
    unitRegistry->addUnitConverter(distanceTypeDef, distanceConverter);
    mx::UnitTypeDefPtr angleTypeDef = _document->getUnitTypeDef("angle");
    mx::LinearUnitConverterPtr angleConverter = mx::LinearUnitConverter::create(angleTypeDef);
    unitRegistry->addUnitConverter(angleTypeDef, angleConverter);

    // Initialize color management.
    mx::ColorManagementSystemPtr cms = mx::DefaultColorManagementSystem::create(
        context.getShaderGenerator().getTarget());
    cms->loadLibrary(_document);
    context.getShaderGenerator().setColorManagementSystem(cms);

    // Initialize unit management.
    mx::UnitSystemPtr unitSystem = mx::UnitSystem::create(context.getShaderGenerator().getTarget());
    unitSystem->loadLibrary(_document);
    unitSystem->setUnitConverterRegistry(unitRegistry);
    context.getShaderGenerator().setUnitSystem(unitSystem);
    context.getOptions().targetDistanceUnit = "meter";

    // Register type definitions.
    context.getShaderGenerator().registerTypeDefs(_document);
}

void CyclesRenderView::generateOsl(mx::TypedElementPtr typedElem)
{
    _oslSource.clear();
    _oslOutputName.clear();

    if (!_document)
    {
        return;
    }

    try
    {
        if (!typedElem)
        {
            std::vector<mx::TypedElementPtr> elements = mx::findRenderableElements(_document);
            if (!elements.empty())
            {
                typedElem = elements[0];
            }
        }

        // Skip material nodes without upstream shaders.
        mx::NodePtr node = typedElem ? typedElem->asA<mx::Node>() : nullptr;
        if (node && node->getCategory() == mx::SURFACE_MATERIAL_NODE_STRING &&
            mx::getShaderNodes(node).empty())
        {
            typedElem = nullptr;
        }

        if (!typedElem)
        {
            return;
        }

        mx::DocumentPtr genDoc = _document->copy();
        mx::FileSearchPath sourceSearchPath = _searchPath;
        sourceSearchPath.append(mx::getSourceSearchPath(_document));
        mx::flattenFilenames(genDoc, sourceSearchPath);
        mx::ElementPtr resolvedElem = genDoc->getDescendant(typedElem->getNamePath());
        mx::TypedElementPtr genElem = resolvedElem ? resolvedElem->asA<mx::TypedElement>() : nullptr;
        if (!genElem)
        {
            genDoc = _document;
            genElem = typedElem;
        }

        _genContext->clearUserData();
        const std::string shaderName = genElem->getNamePath();
        mx::ShaderPtr shader = _genContext->getShaderGenerator().generate(
            shaderName, genElem, *_genContext);

        const mx::ShaderStage& stage = shader->getStage(mx::Stage::PIXEL);
        const mx::VariableBlock& outputs = stage.getOutputBlock(mx::OSL::OUTPUTS);
        if (!outputs.empty())
        {
            _oslOutputName = outputs[0]->getVariable();
            _oslSource = customizeOslForCycles(shader->getSourceCode(mx::Stage::PIXEL));
        }
    }
    catch (mx::Exception& e)
    {
        std::cerr << "Cycles: failed to generate OSL shader: " << e.what() << std::endl;
        _oslSource.clear();
        _oslOutputName.clear();
    }
    catch (std::exception& e)
    {
        std::cerr << "Cycles: failed to generate OSL shader: " << e.what() << std::endl;
        _oslSource.clear();
        _oslOutputName.clear();
    }
}

void CyclesRenderView::rebuildMaterial()
{
    if (!_session)
    {
        return;
    }

    ccl::Scene* scene = _session->scene.get();

    if (_oslSource.empty())
    {
        // No valid material; keep the placeholder surface.
        return;
    }

    // Write the generated OSL to a file that Cycles can compile. A fresh
    // filename is used for each revision so that Cycles' shader cache does not
    // return a stale result.
    const mx::FilePath outputDir = mx::FilePath::getCurrentPath() / "CyclesRenderView";
    outputDir.createDirectory(true);

    // Remove generated shaders from previous revisions.
    for (const mx::FilePath& existing : outputDir.getFilesInDirectory())
    {
        if (existing.getBaseName().rfind("material_", 0) == 0)
        {
            std::remove((outputDir / existing).asString().c_str());
        }
    }

    const mx::FilePath oslPath = outputDir / ("material_" + std::to_string(_materialVersion++) + ".osl");

    std::ofstream output(oslPath.asString());
    if (!output)
    {
        std::cerr << "Cycles: failed to write OSL shader to " << oslPath.asString() << std::endl;
        return;
    }
    output << _oslSource;
    output.close();

    auto graph = std::make_unique<ccl::ShaderGraph>();
    ccl::OSLNode* oslNode = ccl::OSLShaderManager::osl_node(graph.get(), scene, oslPath.asString());
    if (!oslNode)
    {
        std::cerr << "Cycles: failed to load OSL shader " << oslPath.asString() << std::endl;
        return;
    }

    ccl::ShaderOutput* outputSocket = nullptr;
    if (!_oslOutputName.empty())
    {
        outputSocket = oslNode->output(_oslOutputName.c_str());
    }
    if (!outputSocket && !oslNode->outputs.empty())
    {
        outputSocket = oslNode->outputs[0];
    }
    if (!outputSocket)
    {
        std::cerr << "Cycles: OSL shader has no usable output" << std::endl;
        return;
    }

    graph->connect(outputSocket, graph->output()->input("Surface"));
    scene->default_surface->set_graph(std::move(graph));
    scene->default_surface->tag_update(scene);

    restartRender();
}

void CyclesRenderView::loadMesh(const mx::FilePath& /*filename*/)
{
    // The Cycles render view renders the geometry configured at construction.
}

bool CyclesRenderView::isNodeDefSupported(const mx::NodeDefPtr& /*nodeDef*/)
{
    return true;
}

void CyclesRenderView::modifyUniform(const std::string& /*name*/, mx::ValuePtr /*value*/)
{
    // The graph editor has already written the new value into the document, so
    // schedule a regeneration of the OSL shader. Regeneration is debounced so
    // that dragging a slider triggers a single recompilation.
    _materialDirty = true;
    _materialImmediate = false;
    _materialDirtyTime = currentTimeSeconds();
}

void CyclesRenderView::requestFrameCapture(const mx::FilePath& filename)
{
    _captureRequested = true;
    _captureFilename = filename;
    _captureRequestTime = std::chrono::duration<double>(
                              std::chrono::steady_clock::now().time_since_epoch())
                              .count();
}

mx::Vector3 CyclesRenderView::computeCameraEye() const
{
    mx::Matrix44 invArcball = _viewCamera->arcballMatrix().getInverse();
    mx::Vector3 offset = invArcball.transformVector(_cameraPosition - _cameraTarget);
    offset = offset * (1.0f / _cameraZoom);
    return _cameraTarget + offset;
}

mx::Vector3 CyclesRenderView::computeCameraUp() const
{
    mx::Matrix44 invArcball = _viewCamera->arcballMatrix().getInverse();
    return invArcball.transformVector(_cameraUp);
}

void CyclesRenderView::setMouseButtonEvent(int button, bool down, mx::Vector2 pos)
{
    if ((button == 0) && !ImGui::IsKeyPressed(ImGuiKey_RightShift) && !ImGui::IsKeyPressed(ImGuiKey_LeftShift))
    {
        _viewCamera->arcballButtonEvent(pos, down);
        _cameraInteracting = down;
    }
    else if ((button == 1) || ((button == 0) && ImGui::IsKeyDown(ImGuiKey_RightShift)) || ((button == 0) && ImGui::IsKeyDown(ImGuiKey_LeftShift)))
    {
        _userTranslationStart = _userTranslation;
        _userTranslationActive = true;
        _userTranslationPixel = pos;
        _cameraInteracting = down;
    }
    if ((button == 0) && !down)
    {
        _viewCamera->arcballButtonEvent(pos, false);
    }
    if (!down)
    {
        _userTranslationActive = false;
        _cameraInteracting = false;
        _cameraDirty = true;
    }
}

void CyclesRenderView::setMouseMotionEvent(mx::Vector2 pos)
{
    if (_viewCamera->applyArcballMotion(pos))
    {
        _cameraDirty = true;
        return;
    }

    if (_userTranslationActive)
    {
        mx::Vector3 eye = computeCameraEye();
        mx::Vector3 lookAt = _cameraTarget;
        mx::Vector3 forward = (lookAt - eye).getNormalized();
        mx::Vector3 right = forward.cross(computeCameraUp()).getNormalized();
        mx::Vector3 camUp = right.cross(forward).getNormalized();

        float distance = (eye - lookAt).getMagnitude();
        float panScale = 2.0f * distance * std::tan(_cameraFov * (float) PI / 360.0f) /
                         std::max(1.0f, (float) _viewHeight);
        float dx = pos[0] - _userTranslationPixel[0];
        float dy = pos[1] - _userTranslationPixel[1];
        _userTranslation = _userTranslationStart + (right * dx - camUp * dy) * panScale;
        _cameraDirty = true;
    }
}

void CyclesRenderView::setScrollEvent(float scrollY)
{
    _cameraZoom = std::max(0.1f, _cameraZoom * ((scrollY > 0) ? 1.1f : 0.9f));
    _cameraDirty = true;
}

void CyclesRenderView::setKeyEvent(int key)
{
    if (key == ImGuiKey_KeypadAdd)
    {
        _cameraZoom *= 1.1f;
        _cameraDirty = true;
    }
    if (key == ImGuiKey_KeypadSubtract)
    {
        _cameraZoom = std::max(0.1f, _cameraZoom * 0.9f);
        _cameraDirty = true;
    }
}

#endif // MATERIALX_BUILD_RENDER_CYCLES
