//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslRenderer.h>
#include <MaterialXRenderHlsl/HlslRenderUtil.h>

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

// Route the shared binding helpers into an HlslMaterial. patchVariable
// searches every cbuffer of the stage, so this works both in default mode
// (one stage-named cbuffer) and with a binding context that splits
// uniforms across PrivateUniforms / PublicUniforms / LightData.
class MaterialUniformWriter : public HlslUniformWriter
{
  public:
    explicit MaterialUniformWriter(HlslMaterial& material) :
        _material(material)
    {
    }

    bool writeVertexUniform(const std::string& name, const void* data, std::size_t count) override
    {
        return _material.patchVariable(HlslMaterial::Stage::Vertex, name, data, count);
    }

    bool writePixelUniform(const std::string& name, const void* data, std::size_t count) override
    {
        return _material.patchVariable(HlslMaterial::Stage::Pixel, name, data, count);
    }

    bool writePixelArrayMember(const std::string& arrayName, std::size_t index,
                               const std::string& memberName, const void* data, std::size_t count) override
    {
        return _material.patchArrayMember(HlslMaterial::Stage::Pixel, arrayName, index, memberName, data, count);
    }

  private:
    HlslMaterial& _material;
};

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
    MaterialUniformWriter writer(*_material);
    bindHlslCamera(writer, _camera);
}

void HlslRenderer::bindFileTexturesFromImageHandler()
{
    if (!_material || !_imageHandler || !_shader)
        return;
    bindHlslFileTextures(_shader, _imageHandler,
                         [this](const std::string& name, ImagePtr image, const ImageSamplingProperties* sp)
                         {
                             return bindImage(name, image, sp);
                         });
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

void HlslRenderer::renderTextureSpace(const Vector2& /*uvMin*/, const Vector2& /*uvMax*/)
{
    // Texture-space rendering for the baker. The baker calls this with
    // (0,0)/(1,1) by default, which is exactly what the existing
    // fullscreen-triangle path produces. To force that path we
    // temporarily detach the geometry handler so render() falls back
    // to the fullscreen triangle, then restore it. Non-default UV
    // ranges are not yet honoured - `getTextureSpaceMin/Max` would
    // need to thread through to a quad with custom UVs; the baker
    // doesn't currently set them.
    GeometryHandlerPtr saved = _geometryHandler;
    _geometryHandler.reset();
    render();
    _geometryHandler = saved;
}

ImagePtr HlslRenderer::captureImage(ImagePtr image)
{
    if (!_framebuffer)
        return nullptr;
    // Fill the caller's image when it matches the framebuffer, as the
    // TextureBaker captures into its own pre-allocated image.
    return copyHlslCapturedImage(_framebuffer->readColor(), image);
}

void HlslRenderer::bindLightingScalarsFromHandlers()
{
    if (!_material)
        return;
    MaterialUniformWriter writer(*_material);
    bindHlslLightingScalars(writer, _lightHandler);
}

void HlslRenderer::bindLightSourcesFromLightHandler()
{
    if (!_material)
        return;
    MaterialUniformWriter writer(*_material);
    bindHlslLightSources(writer, _lightHandler);
}

void HlslRenderer::bindMaterialUniformsFromShader()
{
    if (!_material || !_shader)
        return;
    MaterialUniformWriter writer(*_material);
    bindHlslMaterialUniforms(writer, _shader);
}

void HlslRenderer::bindEnvironmentImagesFromLightHandler()
{
    if (!_material)
        return;
    bindHlslEnvironmentImages(_lightHandler, _imageHandler,
                              [this](const std::string& name, ImagePtr image, const ImageSamplingProperties* sp)
                              {
                                  return bindImage(name, image, sp);
                              });
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
