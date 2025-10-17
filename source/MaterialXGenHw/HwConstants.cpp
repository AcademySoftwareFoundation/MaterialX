//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenHw/HwConstants.h>
#include <MaterialXGenShader/TypeDesc.h>

MATERIALX_NAMESPACE_BEGIN

namespace HW
{

const string T_IN_POSITION                    = "$inPosition";
const string T_IN_NORMAL                      = "$inNormal";
const string T_IN_TANGENT                     = "$inTangent";
const string T_IN_BITANGENT                   = "$inBitangent";
const string T_IN_TEXCOORD                    = "$inTexcoord";
const string T_IN_GEOMPROP                    = "$inGeomprop";
const string T_IN_COLOR                       = "$inColor";
const string T_POSITION_WORLD                 = "$positionWorld";
const string T_NORMAL_WORLD                   = "$normalWorld";
const string T_TANGENT_WORLD                  = "$tangentWorld";
const string T_BITANGENT_WORLD                = "$bitangentWorld";
const string T_POSITION_OBJECT                = "$positionObject";
const string T_NORMAL_OBJECT                  = "$normalObject";
const string T_TANGENT_OBJECT                 = "$tangentObject";
const string T_BITANGENT_OBJECT               = "$bitangentObject";
const string T_TEXCOORD                       = "$texcoord";
const string T_COLOR                          = "$color";
const string T_WORLD_MATRIX                   = "$worldMatrix";
const string T_WORLD_INVERSE_MATRIX           = "$worldInverseMatrix";
const string T_WORLD_TRANSPOSE_MATRIX         = "$worldTransposeMatrix";
const string T_WORLD_INVERSE_TRANSPOSE_MATRIX = "$worldInverseTransposeMatrix";
const string T_VIEW_MATRIX                    = "$viewMatrix";
const string T_VIEW_INVERSE_MATRIX            = "$viewInverseMatrix";
const string T_VIEW_TRANSPOSE_MATRIX          = "$viewTransposeMatrix";
const string T_VIEW_INVERSE_TRANSPOSE_MATRIX  = "$viewInverseTransposeMatrix";
const string T_PROJ_MATRIX                    = "$projectionMatrix";
const string T_PROJ_INVERSE_MATRIX            = "$projectionInverseMatrix";
const string T_PROJ_TRANSPOSE_MATRIX          = "$projectionTransposeMatrix";
const string T_PROJ_INVERSE_TRANSPOSE_MATRIX  = "$projectionInverseTransposeMatrix";
const string T_WORLD_VIEW_MATRIX              = "$worldViewMatrix";
const string T_VIEW_PROJECTION_MATRIX         = "$viewProjectionMatrix";
const string T_WORLD_VIEW_PROJECTION_MATRIX   = "$worldViewProjectionMatrix";
const string T_VIEW_POSITION                  = "$viewPosition";
const string T_VIEW_DIRECTION                 = "$viewDirection";
const string T_FRAME                          = "$frame";
const string T_TIME                           = "$time";
const string T_GEOMPROP                       = "$geomprop";
const string T_ALPHA_THRESHOLD                = "$alphaThreshold";
const string T_NUM_ACTIVE_LIGHT_SOURCES       = "$numActiveLightSources";
const string T_ENV_MATRIX                     = "$envMatrix";
const string T_ENV_RADIANCE                   = "$envRadiance";
const string T_ENV_RADIANCE_SAMPLER2D         = "$envRadianceSampler2D";
const string T_ENV_RADIANCE_MIPS              = "$envRadianceMips";
const string T_ENV_RADIANCE_SAMPLES           = "$envRadianceSamples";
const string T_ENV_IRRADIANCE                 = "$envIrradiance";
const string T_ENV_IRRADIANCE_SAMPLER2D       = "$envIrradianceSampler2D";
const string T_TEX_SAMPLER_SAMPLER2D          = "$texSamplerSampler2D";
const string T_TEX_SAMPLER_SIGNATURE          = "$texSamplerSignature";
const string T_CLOSURE_DATA_CONSTRUCTOR       = "$closureDataConstructor";

const string T_ENV_LIGHT_INTENSITY            = "$envLightIntensity";
const string T_ENV_PREFILTER_MIP              = "$envPrefilterMip";
const string T_REFRACTION_TWO_SIDED           = "$refractionTwoSided";
const string T_ALBEDO_TABLE                   = "$albedoTable";
const string T_ALBEDO_TABLE_SIZE              = "$albedoTableSize";
const string T_AMB_OCC_MAP                    = "$ambOccMap";
const string T_AMB_OCC_GAIN                   = "$ambOccGain";
const string T_SHADOW_MAP                     = "$shadowMap";
const string T_SHADOW_MATRIX                  = "$shadowMatrix";
const string T_VERTEX_DATA_INSTANCE           = "$vd";
const string T_LIGHT_DATA_INSTANCE            = "$lightData";

const string IN_POSITION                      = "i_position";
const string IN_NORMAL                        = "i_normal";
const string IN_TANGENT                       = "i_tangent";
const string IN_BITANGENT                     = "i_bitangent";
const string IN_TEXCOORD                      = "i_texcoord";
const string IN_GEOMPROP                      = "i_geomprop";
const string IN_COLOR                         = "i_color";
const string POSITION_WORLD                   = "positionWorld";
const string NORMAL_WORLD                     = "normalWorld";
const string TANGENT_WORLD                    = "tangentWorld";
const string BITANGENT_WORLD                  = "bitangentWorld";
const string POSITION_OBJECT                  = "positionObject";
const string NORMAL_OBJECT                    = "normalObject";
const string TANGENT_OBJECT                   = "tangentObject";
const string BITANGENT_OBJECT                 = "bitangentObject";
const string TEXCOORD                         = "texcoord";
const string COLOR                            = "color";
const string WORLD_MATRIX                     = "u_worldMatrix";
const string WORLD_INVERSE_MATRIX             = "u_worldInverseMatrix";
const string WORLD_TRANSPOSE_MATRIX           = "u_worldTransposeMatrix";
const string WORLD_INVERSE_TRANSPOSE_MATRIX   = "u_worldInverseTransposeMatrix";
const string VIEW_MATRIX                      = "u_viewMatrix";
const string VIEW_INVERSE_MATRIX              = "u_viewInverseMatrix";
const string VIEW_TRANSPOSE_MATRIX            = "u_viewTransposeMatrix";
const string VIEW_INVERSE_TRANSPOSE_MATRIX    = "u_viewInverseTransposeMatrix";
const string PROJ_MATRIX                      = "u_projectionMatrix";
const string PROJ_INVERSE_MATRIX              = "u_projectionInverseMatrix";
const string PROJ_TRANSPOSE_MATRIX            = "u_projectionTransposeMatrix";
const string PROJ_INVERSE_TRANSPOSE_MATRIX    = "u_projectionInverseTransposeMatrix";
const string WORLD_VIEW_MATRIX                = "u_worldViewMatrix";
const string VIEW_PROJECTION_MATRIX           = "u_viewProjectionMatrix";
const string WORLD_VIEW_PROJECTION_MATRIX     = "u_worldViewProjectionMatrix";
const string VIEW_POSITION                    = "u_viewPosition";
const string VIEW_DIRECTION                   = "u_viewDirection";
const string FRAME                            = "u_frame";
const string TIME                             = "u_time";
const string GEOMPROP                         = "u_geomprop";
const string ALPHA_THRESHOLD                  = "u_alphaThreshold";
const string NUM_ACTIVE_LIGHT_SOURCES         = "u_numActiveLightSources";
const string ENV_MATRIX                       = "u_envMatrix";
const string ENV_RADIANCE                     = "u_envRadiance";
const string ENV_RADIANCE_SPLIT               = "u_envRadiance_texture, u_envRadiance_sampler";
const string ENV_RADIANCE_SAMPLER2D           = "u_envRadiance";
const string ENV_RADIANCE_SAMPLER2D_SPLIT     = "sampler2D(u_envRadiance_texture, u_envRadiance_sampler)";
const string ENV_RADIANCE_MIPS                = "u_envRadianceMips";
const string ENV_RADIANCE_SAMPLES             = "u_envRadianceSamples";
const string ENV_IRRADIANCE                   = "u_envIrradiance";
const string ENV_IRRADIANCE_SPLIT             = "u_envIrradiance_texture, u_envIrradiance_sampler";
const string ENV_IRRADIANCE_SAMPLER2D         = "u_envIrradiance";
const string ENV_IRRADIANCE_SAMPLER2D_SPLIT   = "sampler2D(u_envIradiance_texture, u_envIrradiance_sampler)";

const string TEX_SAMPLER_SAMPLER2D            = "tex_sampler";
const string TEX_SAMPLER_SAMPLER2D_SPLIT      = "sampler2D(tex_texture, tex_sampler)";
const string TEX_SAMPLER_SAMPLER2D_MSL        = "tex_sampler";
const string TEX_SAMPLER_SIGNATURE            = "sampler2D tex_sampler";
const string TEX_SAMPLER_SIGNATURE_SPLIT      = "texture2D tex_texture, sampler tex_sampler";
const string TEX_SAMPLER_SIGNATURE_MSL        = "MetalTexture tex_sampler";

const string ENV_LIGHT_INTENSITY              = "u_envLightIntensity";
const string ENV_PREFILTER_MIP                = "u_envPrefilterMip";
const string REFRACTION_TWO_SIDED             = "u_refractionTwoSided";
const string ALBEDO_TABLE                     = "u_albedoTable";
const string ALBEDO_TABLE_SIZE                = "u_albedoTableSize";
const string AMB_OCC_MAP                      = "u_ambOccMap";
const string AMB_OCC_GAIN                     = "u_ambOccGain";
const string SHADOW_MAP                       = "u_shadowMap";
const string SHADOW_MATRIX                    = "u_shadowMatrix";
const string VERTEX_DATA_INSTANCE             = "vd";
const string LIGHT_DATA_INSTANCE              = "u_lightData";
const string LIGHT_DATA_MAX_LIGHT_SOURCES     = "MAX_LIGHT_SOURCES";

const string VERTEX_INPUTS                    = "VertexInputs";
const string VERTEX_DATA                      = "VertexData";
const string PRIVATE_UNIFORMS                 = "PrivateUniforms";
const string PUBLIC_UNIFORMS                  = "PublicUniforms";
const string LIGHT_DATA                       = "LightData";
const string PIXEL_OUTPUTS                    = "PixelOutputs";
const string DIR_N                            = "N";
const string CLOSURE_DATA_TYPE                = "ClosureData";
const string CLOSURE_DATA_ARG                 = "closureData";
const string DIR_L                            = "L";
const string DIR_V                            = "V";
const string WORLD_POSITION                   = "P";
const string OCCLUSION                        = "occlusion";
const string CLOSURE_DATA_CONSTRUCTOR         = "ClosureData(closureType, L, V, N, P, occlusion)";
const string ATTR_TRANSPARENT                 = "transparent";
const string USER_DATA_CLOSURE_CONTEXT        = "udcc";
const string USER_DATA_LIGHT_SHADERS          = "udls";
const string USER_DATA_BINDING_CONTEXT        = "udbinding";

const TypeDesc ClosureDataType                = TypeDesc("ClosureData", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_NONE, 1, 0);

const TypedValue<Vector2> VEC2_ZERO            = TypedValue(Vector2(0.f, 0.f));
const TypedValue<Vector2> VEC2_ONE             = TypedValue(Vector2(1.f, 1.f));
const TypedValue<Vector3> VEC3_ZERO            = TypedValue(Vector3(0.f, 0.f, 0.f));
const TypedValue<Vector3> VEC3_ONE             = TypedValue(Vector3(1.f, 1.f, 1.f));

} // namespace HW

MATERIALX_NAMESPACE_END
