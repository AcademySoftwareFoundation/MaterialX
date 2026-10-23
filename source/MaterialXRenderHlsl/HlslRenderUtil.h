//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLRENDERUTIL_H
#define MATERIALX_HLSLRENDERUTIL_H

/// @file
/// Helpers shared by the D3D11 and D3D12 HLSL renderers: uniform binding
/// from MaterialX handlers, and vertex layouts driven by shader reflection.

#include <MaterialXRenderHlsl/Export.h>
#include <MaterialXRenderHlsl/HlslShaderReflection.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXRender/Camera.h>
#include <MaterialXRender/ImageHandler.h>
#include <MaterialXRender/LightHandler.h>
#include <MaterialXRender/Mesh.h>

#include <functional>

MATERIALX_NAMESPACE_BEGIN

/// @class HlslUniformWriter
/// Destination for the uniform values written by the shared binding
/// helpers below. Each renderer adapts its material class to it.
class MX_RENDERHLSL_API HlslUniformWriter
{
  public:
    virtual ~HlslUniformWriter() = default;

    /// Write `count` bytes into the named vertex-stage uniform.
    virtual bool writeVertexUniform(const std::string& name, const void* data, std::size_t count) = 0;

    /// Write `count` bytes into the named pixel-stage uniform.
    virtual bool writePixelUniform(const std::string& name, const void* data, std::size_t count) = 0;

    /// Write `count` bytes into `<arrayName>[index].<memberName>` on the pixel stage.
    virtual bool writePixelArrayMember(const std::string& arrayName, std::size_t index,
                                       const std::string& memberName,
                                       const void* data, std::size_t count) = 0;
};

/// Callback that binds an image to the texture and sampler of a
/// SamplerTexture2D uniform. Returns true if the uniform was found.
using HlslImageBinder = std::function<bool(const std::string& uniformName, ImagePtr image,
                                           const ImageSamplingProperties* properties)>;

/// Pack a MaterialX value into the byte layout HLSL uses for a constant
/// buffer member. Returns the number of bytes written to `out`, or 0 for
/// value types without a packed representation.
MX_RENDERHLSL_API std::size_t packHlslUniformValue(ConstValuePtr value, uint8_t out[64]);

/// Write the values of the pixel-stage public uniforms of `shader`.
/// FILENAME uniforms are skipped; they are bound as textures.
MX_RENDERHLSL_API void bindHlslMaterialUniforms(HlslUniformWriter& writer, ShaderPtr shader);

/// Write the world, view-projection and normal matrices and the view
/// position and direction of `camera`. The projection is remapped from the
/// OpenGL clip-space depth range used by MaterialX cameras to the D3D one,
/// and matrices are transposed for the default column-major cbuffer layout.
MX_RENDERHLSL_API void bindHlslCamera(HlslUniformWriter& writer, CameraPtr camera);

/// Write the environment scalars (matrix, intensity, sample and mip counts,
/// refraction two-sidedness) and the active light count.
MX_RENDERHLSL_API void bindHlslLightingScalars(HlslUniformWriter& writer, LightHandlerPtr lightHandler);

/// Write the type id and input values of every light source of the light
/// handler into the u_lightData array.
MX_RENDERHLSL_API void bindHlslLightSources(HlslUniformWriter& writer, LightHandlerPtr lightHandler);

/// Acquire the image of every FILENAME public uniform of `shader` through
/// `imageHandler` and pass it to `binder` with the sampling properties
/// declared by the uniform's sibling inputs. Lighting textures are skipped.
MX_RENDERHLSL_API void bindHlslFileTextures(ShaderPtr shader, ImageHandlerPtr imageHandler,
                                            const HlslImageBinder& binder);

/// Bind the environment radiance and irradiance maps of the light handler,
/// falling back to the image handler's zero image so that shaders never
/// sample an unbound texture.
MX_RENDERHLSL_API void bindHlslEnvironmentImages(LightHandlerPtr lightHandler, ImageHandlerPtr imageHandler,
                                                 const HlslImageBinder& binder);

/// Give `image` a unique, non-zero resource id if it has none. Images are
/// created with id 0, so texture caches keyed by resource id would
/// otherwise alias every image to the first one uploaded. The counter is
/// shared by the D3D11 and D3D12 texture handlers.
MX_RENDERHLSL_API void assignHlslImageResourceId(ImagePtr image);

/// Copy `source` into `destination` when both images have the same size,
/// channel count and base type, and return `destination`. Otherwise return
/// `source`. Lets renderers honor ShaderRenderer::captureImage(image).
MX_RENDERHLSL_API ImagePtr copyHlslCapturedImage(ImagePtr source, ImagePtr destination);

/// One element of an interleaved vertex layout.
struct HlslVertexElement
{
    std::string semanticName;
    unsigned int semanticIndex = 0;
    unsigned int componentCount = 0;
    HlslComponentType componentType = HlslComponentType::Float;
    unsigned int offset = 0; ///< Byte offset within the vertex.
};

/// Build an interleaved layout holding every vertex shader input, packed as
/// 32-bit components in declaration order. Returns the vertex stride in bytes.
MX_RENDERHLSL_API unsigned int buildHlslVertexLayout(const std::vector<HlslInputParameter>& inputs,
                                                     std::vector<HlslVertexElement>& elements);

/// Interleave the streams of `mesh` into the given layout. POSITION, NORMAL,
/// TANGENT, BINORMAL, TEXCOORD<n> and COLOR<n> read the matching mesh
/// streams. Missing streams and other semantics are zero-filled.
MX_RENDERHLSL_API std::vector<float> interleaveHlslVertices(MeshPtr mesh,
                                                            const std::vector<HlslVertexElement>& elements,
                                                            unsigned int strideBytes);

/// Create a mesh holding a single triangle that covers the whole viewport,
/// with POSITION, NORMAL, TANGENT and TEXCOORD0 streams. Texture
/// coordinates span uvMin to uvMax across the visible area.
MX_RENDERHLSL_API MeshPtr createHlslFullscreenMesh(const Vector2& uvMin = Vector2(0.0f, 0.0f),
                                                   const Vector2& uvMax = Vector2(1.0f, 1.0f));

MATERIALX_NAMESPACE_END

#endif
