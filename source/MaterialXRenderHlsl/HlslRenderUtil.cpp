//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslRenderUtil.h>

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenShader/ShaderStage.h>

#include <array>
#include <atomic>
#include <cstring>

MATERIALX_NAMESPACE_BEGIN

std::size_t packHlslUniformValue(ConstValuePtr value, uint8_t out[64])
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
        // HLSL constant buffers store matrices column-major by default, so
        // the transpose is uploaded for HLSL code to see the MaterialX
        // matrix and compute the same row-vector products as GLSL.
        const Matrix44 m = value->asA<Matrix44>().getTranspose();
        std::memcpy(out, m.data(), sizeof(float) * 16);
        return sizeof(float) * 16;
    }
    return 0;
}

void bindHlslMaterialUniforms(HlslUniformWriter& writer, ShaderPtr shader)
{
    if (!shader)
        return;
    const ShaderStage& ps = shader->getStage(Stage::PIXEL);
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
        // FILENAME inputs are bound as textures. Closure types and other
        // structural inputs have no packed representation and are skipped
        // by packHlslUniformValue returning 0.
        if (port->getType() == Type::FILENAME)
            continue;
        uint8_t buf[64];
        const std::size_t n = packHlslUniformValue(port->getValue(), buf);
        if (n == 0)
            continue;
        writer.writePixelUniform(port->getName(), buf, n);
    }
}

void bindHlslCamera(HlslUniformWriter& writer, CameraPtr camera)
{
    if (!camera)
        return;

    // Reflected names match what HlslShaderGenerator emits via HW::T_*.
    // Two corrections are applied:
    //
    // (1) Matrix transpose. MaterialX stores matrices row-major and the
    //     HLSL VS uses row-vector math (`mul(vec, M)`). HLSL's default
    //     column-major cbuffer layout would read row-major bytes
    //     transposed, so matrices are transposed at upload time. (This is
    //     equivalent to D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, but keeps the
    //     cbuffer layout that reflection reports for default-compiled
    //     shaders.)
    // (2) Depth range. MaterialX cameras build OpenGL projections, which
    //     produce clip Z in [-w, w]. D3D rasterizers clip against [0, w],
    //     so without a remap vertices in front of the near half of the
    //     frustum are clipped. Composing with z_new = (z + w) / 2 converts
    //     between the two conventions.
    Matrix44 zRemap = Matrix44::IDENTITY;
    zRemap[2][2] = 0.5f;
    zRemap[3][2] = 0.5f;
    const Matrix44 viewProj = camera->getViewMatrix() * (camera->getProjectionMatrix() * zRemap);
    const std::array<std::pair<const char*, Matrix44>, 3> entries = { {
        { "u_worldMatrix",                 camera->getWorldMatrix().getTranspose() },
        { "u_viewProjectionMatrix",        viewProj.getTranspose() },
        { "u_worldInverseTransposeMatrix", camera->getWorldMatrix().getInverse() },
    } };
    for (const auto& e : entries)
    {
        writer.writeVertexUniform(e.first, e.second.data(), sizeof(float) * 16);
    }

    // u_viewPosition / u_viewDirection live on the pixel stage and feed the
    // view-direction calculation (Fresnel, half-vector). Without them the
    // pixel stage computes view vectors from the world origin.
    const Vector3 viewPos = camera->getViewPosition();
    writer.writePixelUniform("u_viewPosition", viewPos.data(), sizeof(float) * 3);
    const Vector3 viewDir = camera->getViewDirection();
    writer.writePixelUniform("u_viewDirection", viewDir.data(), sizeof(float) * 3);
}

void bindHlslLightingScalars(HlslUniformWriter& writer, LightHandlerPtr lightHandler)
{
    // u_numActiveLightSources defaults to 0; only set non-zero when a
    // LightHandler is attached and direct lighting is on.
    int activeLights = 0;
    if (lightHandler && lightHandler->getDirectLighting())
        activeLights = static_cast<int>(lightHandler->getLightSources().size());
    writer.writePixelUniform(HW::NUM_ACTIVE_LIGHT_SOURCES, &activeLights, sizeof(int));

    if (!lightHandler)
        return;

    // Env matrix: standard MaterialX convention is rotateY(PI) *
    // transpose(lightTransform); the GLSL renderer uses exactly this. It
    // is uploaded transposed, as are all matrices (see packHlslUniformValue).
    static const float kPi = 3.14159265358979323846f;
    const Matrix44 envMatrix = Matrix44::createRotationY(kPi) *
                               lightHandler->getLightTransform().getTranspose();
    uint8_t envBytes[64];
    const std::size_t envSize = packHlslUniformValue(Value::createValue(envMatrix), envBytes);
    writer.writePixelUniform(HW::ENV_MATRIX, envBytes, envSize);

    const int   sampleCount    = lightHandler->getEnvSampleCount();
    const float lightIntensity = lightHandler->getEnvLightIntensity();
    const int   refractTwoSide = lightHandler->getRefractionTwoSided();
    writer.writePixelUniform(HW::ENV_RADIANCE_SAMPLES, &sampleCount, sizeof(int));
    writer.writePixelUniform(HW::ENV_LIGHT_INTENSITY, &lightIntensity, sizeof(float));
    writer.writePixelUniform(HW::REFRACTION_TWO_SIDED, &refractTwoSide, sizeof(int));

    // ENV_RADIANCE_MIPS comes from the radiance image, not the handler.
    if (lightHandler->getIndirectLighting())
    {
        ImagePtr rad = lightHandler->getUsePrefilteredMap()
                     ? lightHandler->getEnvPrefilteredMap()
                     : lightHandler->getEnvRadianceMap();
        if (rad)
        {
            const int mips = static_cast<int>(rad->getMaxMipCount());
            writer.writePixelUniform(HW::ENV_RADIANCE_MIPS, &mips, sizeof(int));
        }
    }
}

void bindHlslLightSources(HlslUniformWriter& writer, LightHandlerPtr lightHandler)
{
    if (!lightHandler || !lightHandler->getDirectLighting())
        return;

    const auto& lights = lightHandler->getLightSources();
    if (lights.empty())
        return;

    LightIdMap idMap = lightHandler->computeLightIdMap(lights);

    for (std::size_t i = 0; i < lights.size(); ++i)
    {
        NodePtr light = lights[i];
        if (!light)
            continue;
        NodeDefPtr nodeDef = light->getNodeDef();
        if (!nodeDef)
            continue;

        // Light type id, then each input value on the light node. D3D
        // reflection doesn't expose array element members by composed
        // name, so the writer resolves "<array>[i].<member>" itself.
        {
            auto it = idMap.find(nodeDef->getName());
            const int typeValue = (it != idMap.end()) ? static_cast<int>(it->second) : 0;
            writer.writePixelArrayMember(HW::LIGHT_DATA_INSTANCE, i, "type", &typeValue, sizeof(int));
        }
        for (InputPtr input : light->getInputs())
        {
            if (!input || !input->hasValue())
                continue;
            // Light directions follow the light transform, as in
            // GlslProgram::bindLighting.
            ValuePtr value = input->getValue();
            if (input->getName() == "direction" && value->isA<Vector3>())
                value = Value::createValue(lightHandler->getLightTransform().transformVector(value->asA<Vector3>()));
            uint8_t buf[64];
            const std::size_t n = packHlslUniformValue(value, buf);
            if (n == 0)
                continue;
            writer.writePixelArrayMember(HW::LIGHT_DATA_INSTANCE, i, input->getName(), buf, n);
        }
    }
}

void bindHlslFileTextures(ShaderPtr shader, ImageHandlerPtr imageHandler, const HlslImageBinder& binder)
{
    if (!shader || !imageHandler || !binder)
        return;

    // PUBLIC_UNIFORMS exists on every HW shader stage; the canonical copy
    // lives on the pixel stage.
    const ShaderStage& ps = shader->getStage(Stage::PIXEL);
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
        // Lighting textures are bound from the light handler.
        if (uniformName == HW::ENV_RADIANCE || uniformName == HW::ENV_IRRADIANCE)
            continue;

        ImagePtr image;
        if (port->getValue())
        {
            const std::string filePath = port->getValue()->getValueString();
            if (!filePath.empty())
                image = imageHandler->acquireImage(FilePath(filePath));
        }
        if (!image)
            continue;
        // Pull per-uniform sampling properties (uaddressmode, vaddressmode,
        // filtertype, defaultcolor) from the sibling uniforms so the
        // sampler matches the material's intent; UV-tiled materials need
        // repeat addressing.
        ImageSamplingProperties sp;
        sp.setProperties(uniformName, block);
        binder(uniformName, image, &sp);
    }
}

void bindHlslEnvironmentImages(LightHandlerPtr lightHandler, ImageHandlerPtr imageHandler,
                               const HlslImageBinder& binder)
{
    if (!imageHandler || !binder)
        return;

    ImagePtr radiance;
    ImagePtr irradiance;
    if (lightHandler && lightHandler->getIndirectLighting())
    {
        radiance = lightHandler->getUsePrefilteredMap()
                 ? lightHandler->getEnvPrefilteredMap()
                 : lightHandler->getEnvRadianceMap();
        irradiance = lightHandler->getEnvIrradianceMap();
    }
    if (!radiance)   radiance   = imageHandler->getZeroImage();
    if (!irradiance) irradiance = imageHandler->getZeroImage();

    if (radiance)   binder(HW::ENV_RADIANCE, radiance, nullptr);
    if (irradiance) binder(HW::ENV_IRRADIANCE, irradiance, nullptr);
}

void assignHlslImageResourceId(ImagePtr image)
{
    // Start at 1 to keep 0 reserved as "no resource".
    static std::atomic<unsigned int> nextId{ 1 };
    if (image && image->getResourceId() == 0)
        image->setResourceId(nextId.fetch_add(1));
}

ImagePtr copyHlslCapturedImage(ImagePtr source, ImagePtr destination)
{
    if (!source || !destination || !destination->getResourceBuffer() || !source->getResourceBuffer())
        return source;
    if (destination->getWidth() != source->getWidth() ||
        destination->getHeight() != source->getHeight() ||
        destination->getChannelCount() != source->getChannelCount() ||
        destination->getBaseType() != source->getBaseType())
    {
        return source;
    }
    std::memcpy(destination->getResourceBuffer(), source->getResourceBuffer(),
                static_cast<std::size_t>(source->getWidth()) * source->getHeight() *
                source->getChannelCount() * source->getBaseStride());
    return destination;
}

unsigned int buildHlslVertexLayout(const std::vector<HlslInputParameter>& inputs,
                                   std::vector<HlslVertexElement>& elements)
{
    elements.clear();
    unsigned int offset = 0;
    for (const HlslInputParameter& input : inputs)
    {
        HlslVertexElement e;
        e.semanticName = input.semanticName;
        e.semanticIndex = input.semanticIndex;
        e.componentCount = input.componentCount;
        e.componentType = input.componentType;
        e.offset = offset;
        offset += input.componentCount * 4;
        elements.push_back(e);
    }
    return offset;
}

namespace
{

MeshStreamPtr findStreamForSemantic(MeshPtr mesh, const std::string& semantic, unsigned int index)
{
    if (semantic == "POSITION")
        return mesh->getStream(MeshStream::POSITION_ATTRIBUTE, index);
    if (semantic == "NORMAL")
        return mesh->getStream(MeshStream::NORMAL_ATTRIBUTE, index);
    if (semantic == "TANGENT")
        return mesh->getStream(MeshStream::TANGENT_ATTRIBUTE, index);
    if (semantic == "BINORMAL")
        return mesh->getStream(MeshStream::BITANGENT_ATTRIBUTE, index);
    if (semantic == "TEXCOORD")
        return mesh->getStream(MeshStream::TEXCOORD_ATTRIBUTE, index);
    if (semantic == "COLOR")
        return mesh->getStream(MeshStream::COLOR_ATTRIBUTE, index);
    return nullptr;
}

} // namespace

std::vector<float> interleaveHlslVertices(MeshPtr mesh, const std::vector<HlslVertexElement>& elements,
                                          unsigned int strideBytes)
{
    std::vector<float> interleaved;
    if (!mesh || strideBytes == 0)
        return interleaved;

    const std::size_t vertexCount = mesh->getVertexCount();
    const unsigned int strideFloats = strideBytes / 4;
    interleaved.assign(vertexCount * strideFloats, 0.0f);

    for (const HlslVertexElement& e : elements)
    {
        MeshStreamPtr stream = findStreamForSemantic(mesh, e.semanticName, e.semanticIndex);
        if (!stream)
            continue;
        const MeshFloatBuffer& src = stream->getData();
        const unsigned int srcStride = stream->getStride();
        if (srcStride == 0)
            continue;
        const unsigned int copyCount = std::min(e.componentCount, srcStride);
        const std::size_t n = std::min(vertexCount, src.size() / srcStride);
        const unsigned int dstOffset = e.offset / 4;
        for (std::size_t v = 0; v < n; ++v)
        {
            for (unsigned int c = 0; c < copyCount; ++c)
                interleaved[v * strideFloats + dstOffset + c] = src[v * srcStride + c];
        }
    }
    return interleaved;
}

MeshPtr createHlslFullscreenMesh(const Vector2& uvMin, const Vector2& uvMax)
{
    // An over-sized triangle that covers all of NDC. Its texture
    // coordinates are extrapolated so that uvMin..uvMax span the visible
    // [-1, 1] square.
    const float positions[3][2] = { { -1.0f, -1.0f }, { 3.0f, -1.0f }, { -1.0f, 3.0f } };

    MeshPtr mesh = Mesh::create("HlslFullscreenTriangle");
    MeshStreamPtr position = MeshStream::create("i_position", MeshStream::POSITION_ATTRIBUTE, 0);
    MeshStreamPtr normal = MeshStream::create("i_normal", MeshStream::NORMAL_ATTRIBUTE, 0);
    MeshStreamPtr tangent = MeshStream::create("i_tangent", MeshStream::TANGENT_ATTRIBUTE, 0);
    MeshStreamPtr texcoord = MeshStream::create("i_texcoord_0", MeshStream::TEXCOORD_ATTRIBUTE, 0);
    texcoord->setStride(MeshStream::STRIDE_2D);

    for (const auto& p : positions)
    {
        position->getData().insert(position->getData().end(), { p[0], p[1], 0.0f });
        normal->getData().insert(normal->getData().end(), { 0.0f, 0.0f, 1.0f });
        tangent->getData().insert(tangent->getData().end(), { 1.0f, 0.0f, 0.0f });
        const float s = (p[0] + 1.0f) * 0.5f;
        const float t = (p[1] + 1.0f) * 0.5f;
        texcoord->getData().insert(texcoord->getData().end(),
                                   { uvMin[0] + s * (uvMax[0] - uvMin[0]),
                                     uvMin[1] + t * (uvMax[1] - uvMin[1]) });
    }
    mesh->addStream(position);
    mesh->addStream(normal);
    mesh->addStream(tangent);
    mesh->addStream(texcoord);
    mesh->setVertexCount(3);

    MeshPartitionPtr partition = MeshPartition::create();
    partition->getIndices() = { 0, 1, 2 };
    partition->setFaceCount(1);
    mesh->addPartition(partition);
    return mesh;
}

MATERIALX_NAMESPACE_END
