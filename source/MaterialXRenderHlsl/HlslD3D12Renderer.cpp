//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslD3D12Renderer.h>
#include <MaterialXRenderHlsl/HlslRenderUtil.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXRender/Camera.h>
#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/LightHandler.h>
#include <MaterialXRender/Mesh.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

// Route the shared binding helpers into an HlslD3D12Material.
class MaterialUniformWriter : public HlslUniformWriter
{
  public:
    explicit MaterialUniformWriter(HlslD3D12Material& material) :
        _material(material)
    {
    }

    bool writeVertexUniform(const std::string& name, const void* data, std::size_t count) override
    {
        return _material.patchVariable(HlslD3D12Material::Stage::Vertex, name, data, count);
    }

    bool writePixelUniform(const std::string& name, const void* data, std::size_t count) override
    {
        return _material.patchVariable(HlslD3D12Material::Stage::Pixel, name, data, count);
    }

    bool writePixelArrayMember(const std::string& arrayName, std::size_t index,
                               const std::string& memberName, const void* data, std::size_t count) override
    {
        return _material.patchArrayMember(HlslD3D12Material::Stage::Pixel, arrayName, index, memberName, data, count);
    }

  private:
    HlslD3D12Material& _material;
};

} // namespace

HlslD3D12Renderer::HlslD3D12Renderer(unsigned int width, unsigned int height, Image::BaseType baseType) :
    ShaderRenderer(width, height, baseType),
    _backend(HlslProgram::isDxcAvailable() ? HlslCompilerBackend::Dxc : HlslCompilerBackend::Fxc),
    _shaderModel(HlslProgram::isDxcAvailable() ? "6_0" : "5_0")
{
}

HlslD3D12Renderer::~HlslD3D12Renderer()
{
    // Release GPU objects before the context that owns the device.
    _geometry.reset();
    _material.reset();
    _textureHandler.reset();
    _framebuffer.reset();
}

HlslD3D12RendererPtr HlslD3D12Renderer::create(unsigned int width, unsigned int height, Image::BaseType baseType)
{
    return HlslD3D12RendererPtr(new HlslD3D12Renderer(width, height, baseType));
}

ImageHandlerPtr HlslD3D12Renderer::createImageHandler(ImageLoaderPtr imageLoader)
{
    if (!_context)
        initialize();
    return HlslD3D12TextureHandler::create(_context, imageLoader);
}

void HlslD3D12Renderer::setCompilerBackend(HlslCompilerBackend backend, const std::string& shaderModel)
{
    _backend = backend;
    _shaderModel = shaderModel;
}

void HlslD3D12Renderer::initialize(RenderContextHandle)
{
    if (_context)
        return;
    _context = HlslD3D12Context::create(_preferWarp);
    _framebuffer = HlslD3D12Framebuffer::create(_context, _width, _height, _baseType);
    _textureHandler = HlslD3D12TextureHandler::create(_context, nullptr);
    _geometry = HlslD3D12GeometryCache::create(_context);
}

void HlslD3D12Renderer::setSize(unsigned int width, unsigned int height)
{
    if (width == _width && height == _height && _framebuffer)
        return;
    _width = width;
    _height = height;
    if (_context)
    {
        const bool encodeSrgb = _framebuffer ? _framebuffer->getEncodeSrgb() : true;
        _framebuffer = HlslD3D12Framebuffer::create(_context, _width, _height, _baseType);
        _framebuffer->setEncodeSrgb(encodeSrgb);
    }
}

void HlslD3D12Renderer::createProgram(ShaderPtr shader)
{
    if (!_context)
        initialize();
    _program = HlslProgram::create();
    _program->setCompilerBackend(_backend);
    _program->setShaderModel(_shaderModel);
    if (!_program->build(shader))
        throw ExceptionRenderError("HlslD3D12Renderer::createProgram: HLSL compile failed",
                                   { _program->getCompileLog() });
    _material = HlslD3D12Material::create(_context, _program);
    _shader = shader;
}

void HlslD3D12Renderer::createProgram(const StageMap& stages)
{
    if (!_context)
        initialize();
    auto vs = stages.find(Stage::VERTEX);
    auto ps = stages.find(Stage::PIXEL);
    if (vs == stages.end() || ps == stages.end())
        throw ExceptionRenderError("HlslD3D12Renderer::createProgram: stage map missing vertex or pixel source.");
    _program = HlslProgram::create();
    _program->setCompilerBackend(_backend);
    _program->setShaderModel(_shaderModel);
    if (!_program->build(vs->second, ps->second))
        throw ExceptionRenderError("HlslD3D12Renderer::createProgram: HLSL compile failed",
                                   { _program->getCompileLog() });
    _material = HlslD3D12Material::create(_context, _program);
    _shader.reset();
}

void HlslD3D12Renderer::validateInputs()
{
    if (!_program || !_program->isValid())
        throw ExceptionRenderError("HlslD3D12Renderer::validateInputs: no valid program.");
}

void HlslD3D12Renderer::bindUniformsFromHandlers()
{
    MaterialUniformWriter writer(*_material);
    auto binder = [this](const std::string& name, ImagePtr image, const ImageSamplingProperties* sp)
    {
        return bindImage(name, image, sp);
    };

    // Material uniforms first, so that later writes land on top of them.
    bindHlslMaterialUniforms(writer, _shader);
    bindHlslCamera(writer, _camera);
    if (_imageHandler)
    {
        bindHlslFileTextures(_shader, _imageHandler, binder);
        bindHlslEnvironmentImages(_lightHandler, _imageHandler, binder);
    }
    bindHlslLightingScalars(writer, _lightHandler);
    bindHlslLightSources(writer, _lightHandler);
}

void HlslD3D12Renderer::renderMeshes(const std::vector<MeshPtr>& meshes)
{
    const std::vector<HlslVertexElement>& layout = _material->getVertexLayout();
    const unsigned int stride = _material->getVertexStride();

    // Upload any missing buffers before the frame is recorded.
    std::vector<HlslD3D12DrawBuffers> draws;
    for (const MeshPtr& mesh : meshes)
    {
        if (!mesh)
            continue;
        for (size_t i = 0; i < mesh->getPartitionCount(); ++i)
        {
            HlslD3D12DrawBuffers buffers;
            if (_geometry->getDrawBuffers(mesh, mesh->getPartition(i), layout, stride, buffers))
                draws.push_back(buffers);
        }
    }

    ID3D12GraphicsCommandList* list = _context->beginFrame();
    try
    {
        _framebuffer->bind(list);
        if (_clearOnRender)
            _framebuffer->clear(list, _screenColor);
        _material->bind(list, _framebuffer->getRenderTargetFormat(), _framebuffer->getDepthFormat(), _renderState);
        for (const HlslD3D12DrawBuffers& draw : draws)
            HlslD3D12GeometryCache::draw(list, draw);
    }
    catch (...)
    {
        _context->endFrame();
        throw;
    }
    _context->endFrame();
}

void HlslD3D12Renderer::render()
{
    if (!_material || !_framebuffer)
        throw ExceptionRenderError("HlslD3D12Renderer::render: program / framebuffer not initialised.");

    std::vector<MeshPtr> meshes;
    if (_geometryHandler)
    {
        for (const MeshPtr& mesh : _geometryHandler->getMeshes())
        {
            if (mesh && (_activeMeshes.empty() || _activeMeshes.count(mesh->getName())))
                meshes.push_back(mesh);
        }
    }
    if (meshes.empty())
    {
        // Without geometry, draw a fullscreen triangle so that renderer
        // tests that only set a shader still produce output.
        renderTextureSpace(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f));
        return;
    }

    bindUniformsFromHandlers();
    renderMeshes(meshes);
}

void HlslD3D12Renderer::renderTextureSpace(const Vector2& uvMin, const Vector2& uvMax)
{
    if (!_material || !_framebuffer)
        throw ExceptionRenderError("HlslD3D12Renderer::renderTextureSpace: program / framebuffer not initialised.");

    if (!_fullscreenMesh || _fullscreenUvMin != uvMin || _fullscreenUvMax != uvMax)
    {
        if (_fullscreenMesh)
            _geometry->release(_fullscreenMesh);
        _fullscreenMesh = createHlslFullscreenMesh(uvMin, uvMax);
        _fullscreenUvMin = uvMin;
        _fullscreenUvMax = uvMax;
    }

    // The texture baker calls this directly, without render(), so the
    // handler-driven uniforms are bound here too.
    bindUniformsFromHandlers();
    renderMeshes({ _fullscreenMesh });
}

ImagePtr HlslD3D12Renderer::captureImage(ImagePtr image)
{
    if (!_framebuffer)
        return nullptr;
    return _framebuffer->readColor(image);
}

bool HlslD3D12Renderer::bindImage(const std::string& uniformName, ImagePtr image,
                                  const ImageSamplingProperties* properties)
{
    if (!_material || !_textureHandler || !image)
        return false;

    ImageSamplingProperties defaultProperties;
    if (!_textureHandler->bindImage(image, properties ? *properties : defaultProperties))
        return false;

    const uint64_t srv = _textureHandler->getBoundSrv(image->getResourceId());
    const uint64_t sampler = _textureHandler->getBoundSampler(image->getResourceId());
    if (!srv || !sampler)
        return false;

    // The generator declares each texture as a SamplerTexture2D struct;
    // reflection reports its members as "<name>.tex" and "<name>.samp".
    const std::string texName = uniformName + ".tex";
    const std::string sampName = uniformName + ".samp";
    bool found = false;
    for (const HlslResourceBinding& b : _material->getPixelBindings())
    {
        if (b.space != 0)
            continue;
        if (b.type == HlslResourceType::Texture && b.name == texName)
        {
            _material->setTexture(b.slot, srv);
            found = true;
        }
        else if (b.type == HlslResourceType::Sampler && b.name == sampName)
        {
            _material->setSampler(b.slot, sampler);
            found = true;
        }
    }
    return found;
}

MATERIALX_NAMESPACE_END
