//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HWCONSTANTS_H
#define MATERIALX_HWCONSTANTS_H

/// @file
/// Hardware shader generator constants

#include <MaterialXGenHw/Export.h>

#include <MaterialXCore/Value.h>

MATERIALX_NAMESPACE_BEGIN

/*
The HW shader generators have a number of predefined variables (inputs and uniforms) with binding rules.
When these are used by a shader the application must bind them to the expected data. The following table is
a listing of the variables with a description of what data they should be bound to.

However, different renderers can have different requirements on naming conventions for these variables.
In order to facilitate this the generators will use token substitution for naming the variables. The
first column below shows the token names that should be used in source code before the token substitution
is done. The second row shows the real identifier names that will be used by default after substitution.
An generator can override these identifier names in order to use a custom naming convention for these.
Overriding identifier names is done by changing the entries in the identifiers map given to the function
replaceIdentifiers(), which is handling the token substitution on a shader stage.

----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    TOKEN NAME                          DEFAULT IDENTIFIER NAME             TYPE       BINDING
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Vertex input variables :
    $inPosition                         i_position                          vec3       Vertex position in object space
    $inNormal                           i_normal                            vec3       Vertex normal in object space
    $inTangent                          i_tangent                           vec3       Vertex tangent in object space
    $inBitangent                        i_bitangent                         vec3       Vertex bitangent in object space
    $inTexcoord_N                       i_texcoord_N                        vec2       Vertex texture coordinate for the N:th uv set
    $inColor_N                          i_color_N                           vec4       Vertex color for the N:th color set (RGBA)

Uniform variables :
    $worldMatrix                        u_worldMatrix                       mat4       World transformation
    $worldInverseMatrix                 u_worldInverseMatrix                mat4       World transformation, inverted
    $worldTransposeMatrix               u_worldTransposeMatrix              mat4       World transformation, transposed
    $worldInverseTransposeMatrix        u_worldInverseTransposeMatrix       mat4       World transformation, inverted and transposed
    $viewMatrix                         u_viewMatrix                        mat4       View transformation
    $viewInverseMatrix                  u_viewInverseMatrix                 mat4       View transformation, inverted
    $viewTransposeMatrix                u_viewTransposeMatrix               mat4       View transformation, transposed
    $viewInverseTransposeMatrix         u_viewInverseTransposeMatrix        mat4       View transformation, inverted and transposed
    $projectionMatrix                   u_projectionMatrix                  mat4       Projection transformation
    $projectionInverseMatrix            u_projectionInverseMatrix           mat4       Projection transformation, inverted
    $projectionTransposeMatrix          u_projectionTransposeMatrix         mat4       Projection transformation, transposed
    $projectionInverseTransposeMatrix   u_projectionInverseTransposeMatrix  mat4       Projection transformation, inverted and transposed
    $worldViewMatrix                    u_worldViewMatrix                   mat4       World-view transformation
    $viewProjectionMatrix               u_viewProjectionMatrix              mat4       View-projection transformation
    $worldViewProjectionMatrix          u_worldViewProjectionMatrix         mat4       World-view-projection transformation
    $viewPosition                       u_viewPosition                      vec3       World-space position of the view (camera)
    $viewDirection                      u_viewDirection                     vec3       World-space direction of the view (camera)
    $frame                              u_frame                             float      The current frame number as defined by the host application
    $time                               u_time                              float      The current time in seconds
    $geomprop_<name>                    u_geomprop_<name>                   <type>     A named property of given <type> where <name> is the name of the variable on the geometry
    $numActiveLightSources              u_numActiveLightSources             int        The number of currently active light sources. Note that in shader this is clamped against
                                                                                       the maximum allowed number of lights sources. The maximum number is set by the generation
                                                                                       option GenOptions.hwMaxActiveLightSources.
    $lightData[]                        u_lightData[]                       struct     Array of struct LightData holding parameters for active light sources.
                                                                                       The LightData struct is built dynamically depending on requirements for
                                                                                       bound light shaders.
    $envMatrix                          u_envMatrix                         mat4       Rotation matrix for the environment.
    $envIrradiance                      u_envIrradiance                     sampler2D  Sampler for the texture used for diffuse environment lighting.
    $envIrradianceSampler2D             u_envIrradiance                     sampler2D  For split texture and sampler, takes form of sampler2D(tex, sampler)
    $envRadiance                        u_envRadiance                       sampler2D  Sampler for the texture used for specular environment lighting.
    $envRadianceSampler2D               u_envRadiance                       sampler2D  For split texture and sampler, takes form of sampler2D(tex, sampler)
    $envLightIntensity                  u_envLightIntensity                 float      Linear multiplier for environment lighting
    $envRadianceMips                    u_envRadianceMips                   int        Number of mipmaps used on the specular environment texture.
    $envRadianceSamples                 u_envRadianceSamples                int        Samples to use if Filtered Importance Sampling is used for specular environment lighting.
    $texSamplerSampler2D                tex_sampler                         sampler2D  Texture sampler2D parameter. For split texture and sampler, calls sampler2D(tex_texture, tex_sampler).
    $texSamplerSignature                sampler2D tex_sampler               signature  For function signature declaration.

----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/

/// HW specific identifiers.
namespace HW
{
/// Token identifiers
extern MX_GENHW_API const string T_IN_POSITION;
extern MX_GENHW_API const string T_IN_NORMAL;
extern MX_GENHW_API const string T_IN_TANGENT;
extern MX_GENHW_API const string T_IN_BITANGENT;
extern MX_GENHW_API const string T_IN_TEXCOORD;
extern MX_GENHW_API const string T_IN_GEOMPROP;
extern MX_GENHW_API const string T_IN_COLOR;
extern MX_GENHW_API const string T_POSITION_WORLD;
extern MX_GENHW_API const string T_NORMAL_WORLD;
extern MX_GENHW_API const string T_TANGENT_WORLD;
extern MX_GENHW_API const string T_BITANGENT_WORLD;
extern MX_GENHW_API const string T_POSITION_OBJECT;
extern MX_GENHW_API const string T_NORMAL_OBJECT;
extern MX_GENHW_API const string T_TANGENT_OBJECT;
extern MX_GENHW_API const string T_BITANGENT_OBJECT;
extern MX_GENHW_API const string T_TEXCOORD;
extern MX_GENHW_API const string T_COLOR;
extern MX_GENHW_API const string T_WORLD_MATRIX;
extern MX_GENHW_API const string T_WORLD_INVERSE_MATRIX;
extern MX_GENHW_API const string T_WORLD_TRANSPOSE_MATRIX;
extern MX_GENHW_API const string T_WORLD_INVERSE_TRANSPOSE_MATRIX;
extern MX_GENHW_API const string T_VIEW_MATRIX;
extern MX_GENHW_API const string T_VIEW_INVERSE_MATRIX;
extern MX_GENHW_API const string T_VIEW_TRANSPOSE_MATRIX;
extern MX_GENHW_API const string T_VIEW_INVERSE_TRANSPOSE_MATRIX;
extern MX_GENHW_API const string T_PROJ_MATRIX;
extern MX_GENHW_API const string T_PROJ_INVERSE_MATRIX;
extern MX_GENHW_API const string T_PROJ_TRANSPOSE_MATRIX;
extern MX_GENHW_API const string T_PROJ_INVERSE_TRANSPOSE_MATRIX;
extern MX_GENHW_API const string T_WORLD_VIEW_MATRIX;
extern MX_GENHW_API const string T_VIEW_PROJECTION_MATRIX;
extern MX_GENHW_API const string T_WORLD_VIEW_PROJECTION_MATRIX;
extern MX_GENHW_API const string T_VIEW_POSITION;
extern MX_GENHW_API const string T_VIEW_DIRECTION;
extern MX_GENHW_API const string T_FRAME;
extern MX_GENHW_API const string T_TIME;
extern MX_GENHW_API const string T_GEOMPROP;
extern MX_GENHW_API const string T_ALPHA_THRESHOLD;
extern MX_GENHW_API const string T_NUM_ACTIVE_LIGHT_SOURCES;
extern MX_GENHW_API const string T_ENV_MATRIX;
extern MX_GENHW_API const string T_ENV_RADIANCE;
extern MX_GENHW_API const string T_ENV_RADIANCE_SAMPLER2D;
extern MX_GENHW_API const string T_ENV_RADIANCE_MIPS;
extern MX_GENHW_API const string T_ENV_RADIANCE_SAMPLES;
extern MX_GENHW_API const string T_ENV_IRRADIANCE;
extern MX_GENHW_API const string T_ENV_IRRADIANCE_SAMPLER2D;
extern MX_GENHW_API const string T_ENV_LIGHT_INTENSITY;
extern MX_GENHW_API const string T_ENV_PREFILTER_MIP;
extern MX_GENHW_API const string T_REFRACTION_TWO_SIDED;
extern MX_GENHW_API const string T_ALBEDO_TABLE;
extern MX_GENHW_API const string T_ALBEDO_TABLE_SIZE;
extern MX_GENHW_API const string T_AMB_OCC_MAP;
extern MX_GENHW_API const string T_AMB_OCC_GAIN;
extern MX_GENHW_API const string T_SHADOW_MAP;
extern MX_GENHW_API const string T_SHADOW_MATRIX;
extern MX_GENHW_API const string T_VERTEX_DATA_INSTANCE;
extern MX_GENHW_API const string T_LIGHT_DATA_INSTANCE;
extern MX_GENHW_API const string T_TEX_SAMPLER_SAMPLER2D;
extern MX_GENHW_API const string T_TEX_SAMPLER_SIGNATURE;
extern MX_GENHW_API const string T_CLOSURE_DATA_CONSTRUCTOR;

/// Default names for identifiers.
/// Replacing above tokens in final code.
extern MX_GENHW_API const string IN_POSITION;
extern MX_GENHW_API const string IN_NORMAL;
extern MX_GENHW_API const string IN_TANGENT;
extern MX_GENHW_API const string IN_BITANGENT;
extern MX_GENHW_API const string IN_TEXCOORD;
extern MX_GENHW_API const string IN_GEOMPROP;
extern MX_GENHW_API const string IN_COLOR;
extern MX_GENHW_API const string POSITION_WORLD;
extern MX_GENHW_API const string NORMAL_WORLD;
extern MX_GENHW_API const string TANGENT_WORLD;
extern MX_GENHW_API const string BITANGENT_WORLD;
extern MX_GENHW_API const string POSITION_OBJECT;
extern MX_GENHW_API const string NORMAL_OBJECT;
extern MX_GENHW_API const string TANGENT_OBJECT;
extern MX_GENHW_API const string BITANGENT_OBJECT;
extern MX_GENHW_API const string TEXCOORD;
extern MX_GENHW_API const string COLOR;
extern MX_GENHW_API const string WORLD_MATRIX;
extern MX_GENHW_API const string WORLD_INVERSE_MATRIX;
extern MX_GENHW_API const string WORLD_TRANSPOSE_MATRIX;
extern MX_GENHW_API const string WORLD_INVERSE_TRANSPOSE_MATRIX;
extern MX_GENHW_API const string VIEW_MATRIX;
extern MX_GENHW_API const string VIEW_INVERSE_MATRIX;
extern MX_GENHW_API const string VIEW_TRANSPOSE_MATRIX;
extern MX_GENHW_API const string VIEW_INVERSE_TRANSPOSE_MATRIX;
extern MX_GENHW_API const string PROJ_MATRIX;
extern MX_GENHW_API const string PROJ_INVERSE_MATRIX;
extern MX_GENHW_API const string PROJ_TRANSPOSE_MATRIX;
extern MX_GENHW_API const string PROJ_INVERSE_TRANSPOSE_MATRIX;
extern MX_GENHW_API const string WORLD_VIEW_MATRIX;
extern MX_GENHW_API const string VIEW_PROJECTION_MATRIX;
extern MX_GENHW_API const string WORLD_VIEW_PROJECTION_MATRIX;
extern MX_GENHW_API const string VIEW_POSITION;
extern MX_GENHW_API const string VIEW_DIRECTION;
extern MX_GENHW_API const string FRAME;
extern MX_GENHW_API const string TIME;
extern MX_GENHW_API const string GEOMPROP;
extern MX_GENHW_API const string ALPHA_THRESHOLD;
extern MX_GENHW_API const string NUM_ACTIVE_LIGHT_SOURCES;
extern MX_GENHW_API const string ENV_MATRIX;
extern MX_GENHW_API const string ENV_RADIANCE;
extern MX_GENHW_API const string ENV_RADIANCE_SPLIT;
extern MX_GENHW_API const string ENV_RADIANCE_SAMPLER2D;
extern MX_GENHW_API const string ENV_RADIANCE_SAMPLER2D_SPLIT;
extern MX_GENHW_API const string ENV_RADIANCE_MIPS;
extern MX_GENHW_API const string ENV_RADIANCE_SAMPLES;
extern MX_GENHW_API const string ENV_IRRADIANCE;
extern MX_GENHW_API const string ENV_IRRADIANCE_SPLIT;
extern MX_GENHW_API const string ENV_IRRADIANCE_SAMPLER2D;
extern MX_GENHW_API const string ENV_IRRADIANCE_SAMPLER2D_SPLIT;
extern MX_GENHW_API const string ENV_LIGHT_INTENSITY;
extern MX_GENHW_API const string ENV_PREFILTER_MIP;
extern MX_GENHW_API const string REFRACTION_TWO_SIDED;
extern MX_GENHW_API const string ALBEDO_TABLE;
extern MX_GENHW_API const string ALBEDO_TABLE_SIZE;
extern MX_GENHW_API const string AMB_OCC_MAP;
extern MX_GENHW_API const string AMB_OCC_GAIN;
extern MX_GENHW_API const string SHADOW_MAP;
extern MX_GENHW_API const string SHADOW_MATRIX;
extern MX_GENHW_API const string VERTEX_DATA_INSTANCE;
extern MX_GENHW_API const string LIGHT_DATA_INSTANCE;
extern MX_GENHW_API const string LIGHT_DATA_MAX_LIGHT_SOURCES;

/// Texture sampler parameters (for both combined and separate values)
extern MX_GENHW_API const string TEX_SAMPLER_SAMPLER2D;
extern MX_GENHW_API const string TEX_SAMPLER_SAMPLER2D_SPLIT;
extern MX_GENHW_API const string TEX_SAMPLER_SAMPLER2D_MSL;
extern MX_GENHW_API const string TEX_SAMPLER_SIGNATURE;
extern MX_GENHW_API const string TEX_SAMPLER_SIGNATURE_SPLIT;
extern MX_GENHW_API const string TEX_SAMPLER_SIGNATURE_MSL;

/// Variable blocks names.
extern MX_GENHW_API const string VERTEX_INPUTS;    // Geometric inputs for vertex stage.
extern MX_GENHW_API const string VERTEX_DATA;      // Connector block for data transfer from vertex stage to pixel stage.
extern MX_GENHW_API const string PRIVATE_UNIFORMS; // Uniform inputs set privately by application.
extern MX_GENHW_API const string PUBLIC_UNIFORMS;  // Uniform inputs visible in UI and set by user.
extern MX_GENHW_API const string LIGHT_DATA;       // Uniform inputs for light sources.
extern MX_GENHW_API const string PIXEL_OUTPUTS;    // Outputs from the main/pixel stage.

/// Variable names for lighting parameters.
extern MX_GENHW_API const string CLOSURE_DATA_TYPE;
extern MX_GENHW_API const string CLOSURE_DATA_ARG;
extern MX_GENHW_API const string DIR_N;
extern MX_GENHW_API const string DIR_L;
extern MX_GENHW_API const string DIR_V;
extern MX_GENHW_API const string WORLD_POSITION;
extern MX_GENHW_API const string OCCLUSION;

/// Syntax for constructing ClosureData
extern MX_GENHW_API const string CLOSURE_DATA_CONSTRUCTOR;

/// Attribute names.
extern MX_GENHW_API const string ATTR_TRANSPARENT;

/// User data names.
extern MX_GENHW_API const string USER_DATA_LIGHT_SHADERS;
extern MX_GENHW_API const string USER_DATA_BINDING_CONTEXT;

/// Type Descriptor for closure context data.
extern MX_GENHW_API const TypeDesc ClosureDataType;

/// Constant Values
extern MX_GENHW_API const TypedValue<Vector2> VEC2_ZERO;
extern MX_GENHW_API const TypedValue<Vector2> VEC2_ONE;
extern MX_GENHW_API const TypedValue<Vector3> VEC3_ZERO;
extern MX_GENHW_API const TypedValue<Vector3> VEC3_ONE;

} // namespace HW

MATERIALX_NAMESPACE_END

#endif
