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
    releaseAndNull(reinterpret_cast<IUnknown**>(&_meshIbuf));
    releaseAndNull(reinterpret_cast<IUnknown**>(&_meshVbuf));
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

unsigned int HlslRenderer::ensureMeshGeometry()
{
    if (!_geometryHandler)
        return 0;
    const auto& meshes = _geometryHandler->getMeshes();
    if (meshes.empty())
        return 0;
    MeshPtr mesh = meshes.front();
    if (!mesh || mesh->getPartitionCount() == 0)
        return 0;
    MeshPartitionPtr partition = mesh->getPartition(0);
    if (!partition)
        return 0;

    void* key = partition.get();
    if (key == _meshKey && _meshVbuf && _meshIbuf)
        return _meshIndexCount;

    // Discard the old buffers; new mesh -> new upload.
    releaseAndNull(reinterpret_cast<IUnknown**>(&_meshIbuf));
    releaseAndNull(reinterpret_cast<IUnknown**>(&_meshVbuf));
    _meshIndexCount = 0;

    MeshStreamPtr posStream     = mesh->getStream(MeshStream::POSITION_ATTRIBUTE, 0);
    MeshStreamPtr normalStream  = mesh->getStream(MeshStream::NORMAL_ATTRIBUTE,   0);
    MeshStreamPtr tangentStream = mesh->getStream(MeshStream::TANGENT_ATTRIBUTE,  0);
    MeshStreamPtr uvStream      = mesh->getStream(MeshStream::TEXCOORD_ATTRIBUTE, 0);
    if (!posStream)
        return 0;

    const std::size_t vertexCount = mesh->getVertexCount();
    if (vertexCount == 0)
        return 0;

    // Pack interleaved (POSITION, NORMAL, TANGENT, TEXCOORD0) per vertex
    // into the same 11-float layout the fullscreen triangle uses, so
    // both render paths share an input layout. Missing streams get
    // zero-filled.
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

    ID3D11Device* device = _context->getDevice();

    D3D11_BUFFER_DESC vbd = {};
    vbd.ByteWidth = static_cast<UINT>(interleaved.size() * sizeof(float));
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vbInit = { interleaved.data(), 0, 0 };
    if (FAILED(device->CreateBuffer(&vbd, &vbInit, &_meshVbuf)))
        throw ExceptionRenderError("HlslRenderer: failed to create mesh vertex buffer.");

    const auto& indices = partition->getIndices();
    if (indices.empty())
    {
        releaseAndNull(reinterpret_cast<IUnknown**>(&_meshVbuf));
        return 0;
    }

    D3D11_BUFFER_DESC ibd = {};
    ibd.ByteWidth = static_cast<UINT>(indices.size() * sizeof(uint32_t));
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA ibInit = { indices.data(), 0, 0 };
    if (FAILED(device->CreateBuffer(&ibd, &ibInit, &_meshIbuf)))
    {
        releaseAndNull(reinterpret_cast<IUnknown**>(&_meshVbuf));
        throw ExceptionRenderError("HlslRenderer: failed to create mesh index buffer.");
    }

    _meshKey = key;
    _meshIndexCount = static_cast<unsigned int>(indices.size());
    return _meshIndexCount;
}

void HlslRenderer::bindCameraToVertexCbuffer()
{
    if (!_camera || !_material)
        return;

    // Reflected names match what HlslShaderGenerator emits via HW::T_*.
    // patchVariable searches every vertex-stage cbuffer, so the renderer
    // works in both default mode (one stage-named cbuffer) and the
    // binding-context path that splits uniforms across separate cbuffers.
    const std::array<std::pair<const char*, Matrix44>, 3> entries = { {
        { "u_worldMatrix",                  _camera->getWorldMatrix() },
        { "u_viewProjectionMatrix",         _camera->getViewMatrix() * _camera->getProjectionMatrix() },
        { "u_worldInverseTransposeMatrix",  _camera->getWorldMatrix().getInverse().getTranspose() },
    } };
    for (const auto& e : entries)
    {
        _material->patchVariable(HlslMaterial::Stage::Vertex, e.first,
                                 e.second.data(), sizeof(float) * 16);
    }
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
        bindImage(uniformName, image);
    }
}

void HlslRenderer::render()
{
    if (!_material || !_framebuffer)
        throw ExceptionRenderError("HlslRenderer::render: program / framebuffer not initialised.");

    ensureFullscreenGeometry();
    bindCameraToVertexCbuffer();
    bindFileTexturesFromImageHandler();
    bindEnvironmentImagesFromLightHandler();
    bindLightingScalarsFromHandlers();
    bindLightSourcesFromLightHandler();

    // Prefer real mesh geometry when the geometry handler has any. Fall
    // back to the fullscreen triangle so renderer tests that only set a
    // shader still produce output.
    const unsigned int indexCount = ensureMeshGeometry();
    const bool useMesh = indexCount > 0;

    ID3D11DeviceContext* dc = _context->getDeviceContext();

    _framebuffer->bind();
    _framebuffer->clear(_screenColor);
    dc->RSSetState(_rasterState);

    ID3D11Buffer* vbuf = useMesh ? _meshVbuf : _fullscreenVbuf;
    UINT stride = kFullscreenStride; // Same 9-float layout for both paths.
    UINT offset = 0;
    dc->IASetVertexBuffers(0, 1, &vbuf, &stride, &offset);
    dc->IASetInputLayout(_material->getInputLayout());
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    _material->bind();

    if (useMesh)
    {
        dc->IASetIndexBuffer(_meshIbuf, DXGI_FORMAT_R32_UINT, 0);
        dc->DrawIndexed(indexCount, 0, 0);
    }
    else
    {
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

        const std::string prefix = HW::LIGHT_DATA_INSTANCE + "[" + std::to_string(i) + "]";

        // Light type id. patchVariable looks up the right cbuffer, so
        // this works both with the default single-pixelCB layout and
        // with the binding context's split LightData_pixel cbuffer.
        {
            auto it = idMap.find(nodeDef->getName());
            const int typeValue = (it != idMap.end()) ? static_cast<int>(it->second) : 0;
            _material->patchVariable(HlslMaterial::Stage::Pixel,
                                     prefix + ".type",
                                     &typeValue, sizeof(int));
        }

        // Each input on the light node.
        for (InputPtr input : light->getInputs())
        {
            if (!input || !input->hasValue())
                continue;
            uint8_t buf[64];
            const std::size_t n = valueToBytes(input->getValue(), buf);
            if (n == 0)
                continue;
            _material->patchVariable(HlslMaterial::Stage::Pixel,
                                     prefix + "." + input->getName(),
                                     buf, n);
        }
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

bool HlslRenderer::bindImage(const std::string& uniformName, ImagePtr image)
{
    if (!_material || !_textureHandler || !image)
        return false;

    // Default sampling properties: linear, clamp. Callers that need
    // explicit sampling control can call _textureHandler->bindImage
    // first with custom ImageSamplingProperties, then leave the
    // renderer's bindImage to retrieve the cached SRV/sampler.
    ImageSamplingProperties defaultSP;
    if (!_textureHandler->bindImage(image, defaultSP))
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
