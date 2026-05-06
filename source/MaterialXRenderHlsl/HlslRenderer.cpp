//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslRenderer.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXRender/Camera.h>
#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/ImageHandler.h>
#include <MaterialXRender/LightHandler.h>
#include <MaterialXRender/Mesh.h>

#define NOMINMAX 1
#include <Windows.h>
#include <d3d11.h>

#include <algorithm>
#include <cstring>
#include <vector>

MATERIALX_NAMESPACE_BEGIN

namespace
{

// Vertices for an over-sized triangle that covers all of NDC. Each
// vertex carries the standard MaterialX VS attribute set: POSITION (3)
// + NORMAL (3) + TANGENT (3) + TEXCOORD0 (2) = 11 floats. Materials
// whose VS reads only a subset still bind correctly - extra layout
// elements are ignored when the VS signature doesn't consume them.
const float kFullscreenVerts[] = {
    -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
     3.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f,   2.0f, 0.0f,
    -1.0f,  3.0f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 2.0f,
};
constexpr UINT kVertexStrideFloats = 11;
constexpr UINT kFullscreenStride = kVertexStrideFloats * sizeof(float);

void releaseAndNull(IUnknown** ptr)
{
    if (ptr && *ptr)
    {
        (*ptr)->Release();
        *ptr = nullptr;
    }
}

} // namespace

HlslRenderer::HlslRenderer(unsigned int width, unsigned int height, Image::BaseType baseType) :
    ShaderRenderer(width, height, baseType)
{
}

HlslRenderer::~HlslRenderer()
{
    releaseAndNull(reinterpret_cast<IUnknown**>(&_rasterState));
    releaseAndNull(reinterpret_cast<IUnknown**>(&_fullscreenVbuf));
    for (auto& kv : _meshVbufCache)
        releaseAndNull(reinterpret_cast<IUnknown**>(&kv.second));
    _meshVbufCache.clear();
    for (auto& d : _meshDraws)
        releaseAndNull(reinterpret_cast<IUnknown**>(&d.ibuf));
    _meshDraws.clear();
}

HlslRendererPtr HlslRenderer::create(unsigned int width, unsigned int height, Image::BaseType baseType)
{
    return HlslRendererPtr(new HlslRenderer(width, height, baseType));
}

void HlslRenderer::initialize(RenderContextHandle)
{
    if (_context)
        return;
    _context = HlslContext::create();
    _framebuffer = HlslFramebuffer::create(_context, _width, _height);
    // Default texture handler binds a stb-style image loader. Callers
    // that want an alternate loader can replace it after init via
    // setImageHandler / setTextureHandler.
    _textureHandler = HlslTextureHandler::create(_context, nullptr);
}

void HlslRenderer::setSize(unsigned int width, unsigned int height)
{
    if (width == _width && height == _height && _framebuffer)
        return;
    _width = width;
    _height = height;
    if (_context)
    {
        // Replace the framebuffer in-place; the old one is released by
        // the shared_ptr swap.
        _framebuffer = HlslFramebuffer::create(_context, _width, _height);
    }
}

void HlslRenderer::createProgram(ShaderPtr shader)
{
    if (!_context)
        initialize();
    _program = HlslProgram::create();
    if (!_program->build(shader))
        throw ExceptionRenderError("HlslRenderer::createProgram: HLSL compile failed",
                                   { _program->getCompileLog() });
    _material = HlslMaterial::create(_context, _program);
    _shader = shader;

    // Standard MaterialX VS input signature for a surface shader.
    // POSITION + NORMAL + TANGENT cover the unlit/procedural case;
    // TEXCOORD0 covers materials with file textures or geometric uvs.
    const D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    if (!_material->createInputLayout(layoutDesc, 4))
        throw ExceptionRenderError("HlslRenderer::createProgram: failed to create input layout.");
}

void HlslRenderer::createProgram(const StageMap& stages)
{
    if (!_context)
        initialize();
    _program = HlslProgram::create();
    auto vs = stages.find(Stage::VERTEX);
    auto ps = stages.find(Stage::PIXEL);
    if (vs == stages.end() || ps == stages.end())
        throw ExceptionRenderError("HlslRenderer::createProgram: stage map missing vertex or pixel source.");
    if (!_program->build(vs->second, ps->second))
        throw ExceptionRenderError("HlslRenderer::createProgram: HLSL compile failed",
                                   { _program->getCompileLog() });
    _material = HlslMaterial::create(_context, _program);
    _shader.reset();  // No MaterialX shader available via this path.

    const D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    _material->createInputLayout(layoutDesc, 1);
}

void HlslRenderer::validateInputs()
{
    if (!_program || !_program->isValid())
        throw ExceptionRenderError("HlslRenderer::validateInputs: no valid program.");
}

void HlslRenderer::ensureFullscreenGeometry()
{
    if (_fullscreenVbuf)
        return;
    ID3D11Device* device = _context->getDevice();

    D3D11_BUFFER_DESC vbd = {};
    vbd.ByteWidth = sizeof(kFullscreenVerts);
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA init = { kFullscreenVerts, 0, 0 };
    if (FAILED(device->CreateBuffer(&vbd, &init, &_fullscreenVbuf)))
        throw ExceptionRenderError("HlslRenderer: failed to create fullscreen vertex buffer.");

    D3D11_RASTERIZER_DESC rd = {};
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_NONE;
    rd.DepthClipEnable = TRUE;
    if (FAILED(device->CreateRasterizerState(&rd, &_rasterState)))
        throw ExceptionRenderError("HlslRenderer: failed to create rasterizer state.");
}

bool HlslRenderer::ensureMeshGeometry()
{
    if (!_geometryHandler)
        return false;
    const auto& meshes = _geometryHandler->getMeshes();
    if (meshes.empty())
        return false;

    // Build the (Mesh*, Partition*) sequence that the current scene
    // wants to draw and short-circuit when it matches the cache. When
    // _activeMeshes is non-empty, restrict iteration to meshes whose
    // name is in the set (used for multi-pass shaderball-style renders).
    auto meshActive = [&](const MeshPtr& m)
    {
        return _activeMeshes.empty() || _activeMeshes.count(m->getName()) > 0;
    };
    std::vector<std::pair<void*, void*>> wantKey;
    wantKey.reserve(meshes.size());
    for (const MeshPtr& mesh : meshes)
    {
        if (!mesh || !meshActive(mesh))
            continue;
        for (size_t i = 0; i < mesh->getPartitionCount(); ++i)
        {
            MeshPartitionPtr part = mesh->getPartition(i);
            if (!part)
                continue;
            wantKey.emplace_back(mesh.get(), part.get());
        }
    }
    if (wantKey.empty())
        return false;
    if (wantKey == _meshCacheKey && !_meshDraws.empty())
        return true;

    // Different scene-graph identity: discard owned buffers and rebuild.
    for (auto& kv : _meshVbufCache)
        releaseAndNull(reinterpret_cast<IUnknown**>(&kv.second));
    _meshVbufCache.clear();
    for (auto& d : _meshDraws)
        releaseAndNull(reinterpret_cast<IUnknown**>(&d.ibuf));
    _meshDraws.clear();
    _meshCacheKey.clear();

    ID3D11Device* device = _context->getDevice();

    auto buildVbuf = [&](const MeshPtr& mesh) -> ID3D11Buffer*
    {
        auto it = _meshVbufCache.find(mesh.get());
        if (it != _meshVbufCache.end())
            return it->second;

        MeshStreamPtr posStream     = mesh->getStream(MeshStream::POSITION_ATTRIBUTE, 0);
        MeshStreamPtr normalStream  = mesh->getStream(MeshStream::NORMAL_ATTRIBUTE,   0);
        MeshStreamPtr tangentStream = mesh->getStream(MeshStream::TANGENT_ATTRIBUTE,  0);
        MeshStreamPtr uvStream      = mesh->getStream(MeshStream::TEXCOORD_ATTRIBUTE, 0);
        if (!posStream)
            return nullptr;

        const std::size_t vertexCount = mesh->getVertexCount();
        if (vertexCount == 0)
            return nullptr;

        // Pack interleaved (POSITION, NORMAL, TANGENT, TEXCOORD0) per
        // vertex into the same 11-float layout the fullscreen triangle
        // uses, so both render paths share an input layout. Missing
        // streams get zero-filled.
        constexpr unsigned int kFloatsPerVertex = kVertexStrideFloats;
        std::vector<float> interleaved(vertexCount * kFloatsPerVertex, 0.0f);
        auto writeStream = [&](const MeshStreamPtr& s, unsigned int dstOffset, unsigned int dstWidth)
        {
            if (!s)
                return;
            const auto& src = s->getData();
            const unsigned int srcStride = s->getStride();
            const unsigned int copyN = std::min(dstWidth, srcStride);
            const std::size_t n = std::min(vertexCount, src.size() / (srcStride ? srcStride : 1));
            for (std::size_t i = 0; i < n; ++i)
            {
                for (unsigned int c = 0; c < copyN; ++c)
                    interleaved[i * kFloatsPerVertex + dstOffset + c] = src[i * srcStride + c];
            }
        };
        writeStream(posStream,     0, 3);
        writeStream(normalStream,  3, 3);
        writeStream(tangentStream, 6, 3);
        writeStream(uvStream,      9, 2);

        D3D11_BUFFER_DESC vbd = {};
        vbd.ByteWidth = static_cast<UINT>(interleaved.size() * sizeof(float));
        vbd.Usage = D3D11_USAGE_IMMUTABLE;
        vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA vbInit = { interleaved.data(), 0, 0 };
        ID3D11Buffer* vbuf = nullptr;
        if (FAILED(device->CreateBuffer(&vbd, &vbInit, &vbuf)))
            throw ExceptionRenderError("HlslRenderer: failed to create mesh vertex buffer.");
        _meshVbufCache.emplace(mesh.get(), vbuf);
        return vbuf;
    };

    for (const MeshPtr& mesh : meshes)
    {
        if (!mesh || !meshActive(mesh))
            continue;
        ID3D11Buffer* vbuf = buildVbuf(mesh);
        if (!vbuf)
            continue;

        for (size_t i = 0; i < mesh->getPartitionCount(); ++i)
        {
            MeshPartitionPtr part = mesh->getPartition(i);
            if (!part)
                continue;
            const auto& indices = part->getIndices();
            if (indices.empty())
                continue;

            D3D11_BUFFER_DESC ibd = {};
            ibd.ByteWidth = static_cast<UINT>(indices.size() * sizeof(uint32_t));
            ibd.Usage = D3D11_USAGE_IMMUTABLE;
            ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            D3D11_SUBRESOURCE_DATA ibInit = { indices.data(), 0, 0 };
            ID3D11Buffer* ibuf = nullptr;
            if (FAILED(device->CreateBuffer(&ibd, &ibInit, &ibuf)))
                throw ExceptionRenderError("HlslRenderer: failed to create mesh index buffer.");

            MeshDraw d;
            d.vbuf = vbuf;
            d.ibuf = ibuf;
            d.indexCount = static_cast<unsigned int>(indices.size());
            _meshDraws.push_back(d);
        }
    }

    _meshCacheKey = std::move(wantKey);
    return !_meshDraws.empty();
}

void HlslRenderer::bindCameraToVertexCbuffer()
{
    if (!_camera || !_material)
        return;

    // Reflected names match what HlslShaderGenerator emits via HW::T_*.
    // patchVariable searches every vertex-stage cbuffer, so the renderer
    // works in both default mode (one stage-named cbuffer) and the
    // binding-context path that splits uniforms across separate cbuffers.
    // Two corrections applied here:
    //
    // (1) Matrix transpose. MaterialX stores matrices row-major and the
    //     HLSL VS uses row-vector math (`mul(vec, M)`). HLSL's default
    //     column-major cbuffer layout would read our row-major bytes
    //     transposed - which mangles the projection. Transposing at
    //     upload time gives HLSL bytes whose column-major
    //     interpretation equals MaterialX's intended row-vector matrix.
    //     (Equivalent to the D3DCOMPILE_PACK_MATRIX_ROW_MAJOR flag, but
    //     keeps the cbuffer layout that D3D reflection reports for the
    //     default-compiled shader, so existing reflection-based code
    //     stays correct.)
    // (2) X-flip on the projection. Camera::createPerspectiveMatrix is
    //     a right-handed projection (the GLSL renderer's convention).
    //     D3D11 rasterises in left-handed clip space, so feeding the RH
    //     matrix through unchanged horizontally mirrors the render.
    //     Composing with a diagonal X-flip on the right side of the
    //     projection corrects it, matching what GlslRenderer produces
    //     visually.
    // GL projection produces clip Z in [-w, w] (NDC [-1, 1]).
    // D3D11 rasterizer clips against [0, w] (NDC [0, 1]). Without
    // remapping, vertices with NDC z < 0 (front of sphere) get
    // clipped, leaving only back-facing surfaces visible - which then
    // appear X-mirrored under lighting because their normals point
    // away from the actual lit side of the scene.
    //
    // Compose with a remap matrix that converts z_gl in [-w, w] to
    // z_d3d in [0, w]:  z_new = (z + w) / 2.
    Matrix44 zRemap = Matrix44::IDENTITY;
    zRemap[2][2] = 0.5f;
    zRemap[3][2] = 0.5f;
    Matrix44 proj = _camera->getProjectionMatrix();
    Matrix44 viewProj = _camera->getViewMatrix() * (proj * zRemap);
    const std::array<std::pair<const char*, Matrix44>, 3> entries = { {
        { "u_worldMatrix",                  _camera->getWorldMatrix().getTranspose() },
        { "u_viewProjectionMatrix",         viewProj.getTranspose() },
        { "u_worldInverseTransposeMatrix",  _camera->getWorldMatrix().getInverse() },
    } };
    for (const auto& e : entries)
    {
        _material->patchVariable(HlslMaterial::Stage::Vertex, e.first,
                                 e.second.data(), sizeof(float) * 16);
    }

    // u_viewPosition / u_viewDirection live on the pixel stage and feed
    // the PS view-direction calculation (Fresnel, half-vector for the
    // Cook-Torrance specular). Without them the PS computes view from
    // the world origin, biasing every glossy material's highlight.
    const Vector3 viewPos = _camera->getViewPosition();
    _material->patchVariable(HlslMaterial::Stage::Pixel, "u_viewPosition",
                             viewPos.data(), sizeof(float) * 3);
    const Vector3 viewDir = _camera->getViewDirection();
    _material->patchVariable(HlslMaterial::Stage::Pixel, "u_viewDirection",
                             viewDir.data(), sizeof(float) * 3);
}

void HlslRenderer::bindFileTexturesFromImageHandler()
{
    if (!_material || !_imageHandler || !_shader)
        return;

    // PUBLIC_UNIFORMS exists on every HW shader stage; the canonical
    // copy lives on the pixel stage.
    const ShaderStage& ps = _shader->getStage(Stage::PIXEL);
    const VariableBlockMap& blocks = ps.getUniformBlocks();
    auto it = blocks.find(HW::PUBLIC_UNIFORMS);
    if (it == blocks.end() || !it->second)
        return;
    const VariableBlock& block = *it->second;

    for (size_t i = 0; i < block.size(); ++i)
    {
        const ShaderPort* port = block[i];
        if (!port || port->getType() != Type::FILENAME)
            continue;
        const std::string& uniformName = port->getName();
        // Skip lighting textures; those are bound by future LightHandler
        // integration with their own samplers and filter rules.
        if (uniformName == HW::ENV_RADIANCE || uniformName == HW::ENV_IRRADIANCE)
            continue;

        ImagePtr image;
        if (port->getValue())
        {
            const std::string filePath = port->getValue()->getValueString();
            if (!filePath.empty())
                image = _imageHandler->acquireImage(FilePath(filePath));
        }
        if (!image)
            continue;
        // Pull per-uniform sampling properties (uaddressmode,
        // vaddressmode, filtertype, defaultcolor) from the cbuffer
        // sibling uniforms so the sampler matches the material's
        // intent. Without this, every texture binds with the default
        // CLAMP/LINEAR sampler, which makes UV-tiled materials (e.g.
        // wood with uvtiling=4,4) sample at the texture edge instead
        // of repeating the pattern.
        ImageSamplingProperties sp;
        sp.setProperties(uniformName, block);
        bindImage(uniformName, image, &sp);
    }
}

void HlslRenderer::render()
{
    if (!_material || !_framebuffer)
        throw ExceptionRenderError("HlslRenderer::render: program / framebuffer not initialised.");

    ensureFullscreenGeometry();
    // Material uniforms (base_color, opacity, roughness, ...) must be
    // pushed before the lighting / per-light passes so subsequent
    // patchVariable calls don't see a stale or zero-init cbuffer when
    // they read-modify-write neighbouring members.
    bindMaterialUniformsFromShader();
    bindCameraToVertexCbuffer();
    bindFileTexturesFromImageHandler();
    bindEnvironmentImagesFromLightHandler();
    bindLightingScalarsFromHandlers();
    bindLightSourcesFromLightHandler();

    // Prefer real mesh geometry when the geometry handler has any. Fall
    // back to the fullscreen triangle so renderer tests that only set a
    // shader still produce output.
    const bool useMesh = ensureMeshGeometry();

    ID3D11DeviceContext* dc = _context->getDeviceContext();

    _framebuffer->bind();
    if (_clearOnRender)
        _framebuffer->clear(_screenColor);
    dc->RSSetState(_rasterState);

    UINT stride = kFullscreenStride; // Same 9-float layout for both paths.
    UINT offset = 0;
    dc->IASetInputLayout(_material->getInputLayout());
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    _material->bind();

    if (useMesh)
    {
        // Mirror GlslRenderer: bind each mesh's VBO, then draw every
        // partition of that mesh. The vbuf pointer changes only when
        // we move to a new mesh, but D3D11 doesn't care if we re-bind
        // the same buffer per partition.
        for (const MeshDraw& d : _meshDraws)
        {
            dc->IASetVertexBuffers(0, 1, &d.vbuf, &stride, &offset);
            dc->IASetIndexBuffer(d.ibuf, DXGI_FORMAT_R32_UINT, 0);
            dc->DrawIndexed(d.indexCount, 0, 0);
        }
    }
    else
    {
        dc->IASetVertexBuffers(0, 1, &_fullscreenVbuf, &stride, &offset);
        dc->Draw(3, 0);
    }

    _framebuffer->unbind();
}

ImagePtr HlslRenderer::captureImage(ImagePtr /*image*/)
{
    if (!_framebuffer)
        return nullptr;
    return _framebuffer->readColor();
}

namespace
{

// Patch a single pixel-stage uniform regardless of which reflected
// cbuffer it lives in. patchVariable searches every pixel cbuffer and
// writes into whichever one owns the uniform, so the renderer works
// in both default mode (one stage-named cbuffer) and the binding-
// context mode that splits uniforms across PrivateUniforms /
// PublicUniforms / LightData.
void writePixelCbufferMember(HlslMaterial& mat, const std::string& memberName,
                             const void* bytes, std::size_t count)
{
    mat.patchVariable(HlslMaterial::Stage::Pixel, memberName, bytes, count);
}

} // namespace

void HlslRenderer::bindLightingScalarsFromHandlers()
{
    if (!_material)
        return;

    // u_numActiveLightSources defaults to 0; only set non-zero when a
    // LightHandler is attached and direct lighting is on. Per-light
    // parameter binding lives in a follow-up.
    int activeLights = 0;
    if (_lightHandler && _lightHandler->getDirectLighting())
        activeLights = static_cast<int>(_lightHandler->getLightSources().size());
    writePixelCbufferMember(*_material, HW::NUM_ACTIVE_LIGHT_SOURCES,
                            &activeLights, sizeof(int));

    if (!_lightHandler)
        return;

    // Env matrix: standard MaterialX convention is rotateY(PI) *
    // transpose(lightTransform); the GLSL renderer uses exactly this.
    static const float kPi = 3.14159265358979323846f;
    Matrix44 envMatrix = Matrix44::createRotationY(kPi) *
                         _lightHandler->getLightTransform().getTranspose();
    writePixelCbufferMember(*_material, HW::ENV_MATRIX,
                            envMatrix.data(), sizeof(float) * 16);

    const int   sampleCount    = _lightHandler->getEnvSampleCount();
    const float lightIntensity = _lightHandler->getEnvLightIntensity();
    const int   refractTwoSide = _lightHandler->getRefractionTwoSided();
    writePixelCbufferMember(*_material, HW::ENV_RADIANCE_SAMPLES,
                            &sampleCount, sizeof(int));
    writePixelCbufferMember(*_material, HW::ENV_LIGHT_INTENSITY,
                            &lightIntensity, sizeof(float));
    writePixelCbufferMember(*_material, HW::REFRACTION_TWO_SIDED,
                            &refractTwoSide, sizeof(int));

    // ENV_RADIANCE_MIPS comes from the radiance image, not the handler.
    if (_lightHandler->getIndirectLighting())
    {
        ImagePtr rad = _lightHandler->getUsePrefilteredMap()
                     ? _lightHandler->getEnvPrefilteredMap()
                     : _lightHandler->getEnvRadianceMap();
        if (rad)
        {
            const int mips = static_cast<int>(rad->getMaxMipCount());
            writePixelCbufferMember(*_material, HW::ENV_RADIANCE_MIPS,
                                    &mips, sizeof(int));
        }
    }
}

namespace
{

// Convert a MaterialX value into raw bytes the cbuffer expects. Returns
// the number of bytes written into `out`, or 0 if the value type isn't
// one we know how to pack.
std::size_t valueToBytes(ConstValuePtr value, uint8_t out[64])
{
    if (!value)
        return 0;
    if (value->isA<int>())
    {
        const int v = value->asA<int>();
        std::memcpy(out, &v, sizeof(int));
        return sizeof(int);
    }
    if (value->isA<float>())
    {
        const float v = value->asA<float>();
        std::memcpy(out, &v, sizeof(float));
        return sizeof(float);
    }
    if (value->isA<bool>())
    {
        const int v = value->asA<bool>() ? 1 : 0;  // HLSL bool packs as 4 bytes.
        std::memcpy(out, &v, sizeof(int));
        return sizeof(int);
    }
    if (value->isA<Color3>())
    {
        const Color3& c = value->asA<Color3>();
        std::memcpy(out, c.data(), sizeof(float) * 3);
        return sizeof(float) * 3;
    }
    if (value->isA<Color4>())
    {
        const Color4& c = value->asA<Color4>();
        std::memcpy(out, c.data(), sizeof(float) * 4);
        return sizeof(float) * 4;
    }
    if (value->isA<Vector2>())
    {
        const Vector2& v = value->asA<Vector2>();
        std::memcpy(out, v.data(), sizeof(float) * 2);
        return sizeof(float) * 2;
    }
    if (value->isA<Vector3>())
    {
        const Vector3& v = value->asA<Vector3>();
        std::memcpy(out, v.data(), sizeof(float) * 3);
        return sizeof(float) * 3;
    }
    if (value->isA<Vector4>())
    {
        const Vector4& v = value->asA<Vector4>();
        std::memcpy(out, v.data(), sizeof(float) * 4);
        return sizeof(float) * 4;
    }
    if (value->isA<Matrix44>())
    {
        const Matrix44& m = value->asA<Matrix44>();
        std::memcpy(out, m.data(), sizeof(float) * 16);
        return sizeof(float) * 16;
    }
    return 0;
}

} // namespace

void HlslRenderer::bindLightSourcesFromLightHandler()
{
    if (!_material || !_lightHandler || !_lightHandler->getDirectLighting())
        return;

    const auto& lights = _lightHandler->getLightSources();
    if (lights.empty())
        return;

    LightIdMap idMap = _lightHandler->computeLightIdMap(lights);

    for (std::size_t i = 0; i < lights.size(); ++i)
    {
        NodePtr light = lights[i];
        if (!light)
            continue;
        NodeDefPtr nodeDef = light->getNodeDef();
        if (!nodeDef)
            continue;

        // Light type id, then each input value on the light node.
        // patchArrayMember resolves "<HW::LIGHT_DATA_INSTANCE>[i].<name>"
        // by walking the reflected LightData struct - composing the name
        // and feeding patchVariable would silently fail because D3D
        // reflection doesn't expose array element members by name.
        {
            auto it = idMap.find(nodeDef->getName());
            const int typeValue = (it != idMap.end()) ? static_cast<int>(it->second) : 0;
            _material->patchArrayMember(HlslMaterial::Stage::Pixel,
                                        HW::LIGHT_DATA_INSTANCE, i, "type",
                                        &typeValue, sizeof(int));
        }
        for (InputPtr input : light->getInputs())
        {
            if (!input || !input->hasValue())
                continue;
            uint8_t buf[64];
            const std::size_t n = valueToBytes(input->getValue(), buf);
            if (n == 0)
                continue;
            _material->patchArrayMember(HlslMaterial::Stage::Pixel,
                                        HW::LIGHT_DATA_INSTANCE, i,
                                        input->getName(),
                                        buf, n);
        }
    }
}

void HlslRenderer::bindMaterialUniformsFromShader()
{
    if (!_material || !_shader)
        return;
    const ShaderStage& ps = _shader->getStage(Stage::PIXEL);
    const VariableBlockMap& blocks = ps.getUniformBlocks();
    auto it = blocks.find(HW::PUBLIC_UNIFORMS);
    if (it == blocks.end() || !it->second)
        return;
    const VariableBlock& block = *it->second;
    for (size_t i = 0; i < block.size(); ++i)
    {
        const ShaderPort* port = block[i];
        if (!port || !port->getValue())
            continue;
        // FILENAME inputs are bound as textures elsewhere
        // (bindFileTexturesFromImageHandler). Closure types and other
        // structural inputs (surfaceshader, displacementshader, BSDF,
        // EDF, ...) don't have packable byte representations and will
        // be skipped by valueToBytes returning 0.
        if (port->getType() == Type::FILENAME)
            continue;
        uint8_t buf[64];
        const std::size_t n = valueToBytes(port->getValue(), buf);
        if (n == 0)
            continue;
        _material->patchVariable(HlslMaterial::Stage::Pixel,
                                 port->getName(), buf, n);
    }
}

void HlslRenderer::bindEnvironmentImagesFromLightHandler()
{
    if (!_material || !_imageHandler)
        return;

    // Resolve env images. With a LightHandler that has indirect lighting
    // enabled use its maps; otherwise (and as fallback when those maps
    // are null) use the image handler's stock zero image so the GPU
    // never samples a null SRV.
    ImagePtr radiance;
    ImagePtr irradiance;
    if (_lightHandler && _lightHandler->getIndirectLighting())
    {
        radiance = _lightHandler->getUsePrefilteredMap()
                 ? _lightHandler->getEnvPrefilteredMap()
                 : _lightHandler->getEnvRadianceMap();
        irradiance = _lightHandler->getEnvIrradianceMap();
    }
    if (!radiance)   radiance   = _imageHandler->getZeroImage();
    if (!irradiance) irradiance = _imageHandler->getZeroImage();

    if (radiance)   bindImage(HW::ENV_RADIANCE,   radiance);
    if (irradiance) bindImage(HW::ENV_IRRADIANCE, irradiance);
}

bool HlslRenderer::bindImage(const std::string& uniformName, ImagePtr image,
                             const ImageSamplingProperties* properties)
{
    if (!_material || !_textureHandler || !image)
        return false;

    // Use the supplied per-uniform sampling properties when available;
    // otherwise fall back to defaults (CLAMP / LINEAR). The properties
    // come from the shader's PUBLIC_UNIFORMS sibling uniforms
    // (<file>_uaddressmode etc.) and let UV-tiled materials sample
    // with WRAP, materials with custom filters use point/cubic, and
    // missing-image fallbacks use the requested default colour.
    ImageSamplingProperties defaultSP;
    const ImageSamplingProperties& sp = properties ? *properties : defaultSP;
    if (!_textureHandler->bindImage(image, sp))
        return false;

    ID3D11ShaderResourceView* srv     = _textureHandler->getBoundSrv(image->getResourceId());
    ID3D11SamplerState*       sampler = _textureHandler->getBoundSampler(image->getResourceId());
    if (!srv || !sampler)
        return false;

    const std::string texName  = uniformName + ".tex";
    const std::string sampName = uniformName + ".samp";

    bool foundTex = false;
    bool foundSamp = false;
    for (const auto& b : _material->getPixelBindings())
    {
        if (b.type == HlslResourceType::Texture && b.name == texName)
        {
            _material->setTexture(b.slot, srv);
            foundTex = true;
        }
        else if (b.type == HlslResourceType::Sampler && b.name == sampName)
        {
            _material->setSampler(b.slot, sampler);
            foundSamp = true;
        }
    }
    return foundTex || foundSamp;
}

MATERIALX_NAMESPACE_END
