//Metal Shading Language version 2.3
#define __METAL__ 
#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
#define vec2 float2
#define vec3 float3
#define vec4 float4
#define ivec2 int2
#define ivec3 int3
#define ivec4 int4
#define uvec2 uint2
#define uvec3 uint3
#define uvec4 uint4
#define bvec2 bool2
#define bvec3 bool3
#define bvec4 bool4
#define mat3 float3x3
#define mat4 float4x4


// Uniform block: PrivateUniforms
struct PrivateUniforms
{
    mat4 u_worldMatrix;
    mat4 u_viewProjectionMatrix;
    mat4 u_worldInverseTransposeMatrix;
};

// Inputs block: VertexInputs
struct VertexInputs
{
    vec3 i_position [[attribute(0)]];
    vec2 i_texcoord_0 [[attribute(1)]];
    vec3 i_normal [[attribute(2)]];
    vec3 i_tangent [[attribute(3)]];
};
struct VertexData
{
    float4 pos [[position]];
    vec2 texcoord_0;
    vec3 normalWorld;
    vec3 tangentWorld;
    vec3 positionWorld;
};

struct GlobalContext
{
    GlobalContext(
    vec3 i_position
,     vec2 i_texcoord_0
,     vec3 i_normal
,     vec3 i_tangent
    ,     mat4 u_worldMatrix

    ,     mat4 u_viewProjectionMatrix

    ,     mat4 u_worldInverseTransposeMatrix

    ) : 
    i_position(i_position)
,     i_texcoord_0(i_texcoord_0)
,     i_normal(i_normal)
,     i_tangent(i_tangent)
    ,     u_worldMatrix(u_worldMatrix)

    ,     u_viewProjectionMatrix(u_viewProjectionMatrix)

    ,     u_worldInverseTransposeMatrix(u_worldInverseTransposeMatrix)

    {}
    vec3 i_position;

    vec2 i_texcoord_0;

    vec3 i_normal;

    vec3 i_tangent;
    
    mat4 u_worldMatrix;

    
    mat4 u_viewProjectionMatrix;

    
    mat4 u_worldInverseTransposeMatrix;

    VertexData VertexMain()
    {
        VertexData vd;
        float4 hPositionWorld = u_worldMatrix * float4(i_position, 1.0);
        vd.pos = u_viewProjectionMatrix * hPositionWorld;
        vd.texcoord_0 = i_texcoord_0;
        vd.normalWorld = normalize((u_worldInverseTransposeMatrix * vec4(i_normal, 0.0)).xyz);
        vd.tangentWorld = normalize((u_worldMatrix * vec4(i_tangent, 0.0)).xyz);
        // Omitted node 'N_mult_float'. Function already called in this scope.
        // Omitted node 'N_sub_float'. Function already called in this scope.
        // Omitted node 'N_divtilesize_float'. Function already called in this scope.
        // Omitted node 'N_multtilesize_float'. Function already called in this scope.
        // Omitted node 'N_img_float'. Function already called in this scope.
        // Omitted node 'N_mult_float'. Function already called in this scope.
        // Omitted node 'N_sub_float'. Function already called in this scope.
        // Omitted node 'N_divtilesize_float'. Function already called in this scope.
        // Omitted node 'N_multtilesize_float'. Function already called in this scope.
        // Omitted node 'N_img_float'. Function already called in this scope.
        // Omitted node 'N_mult_float'. Function already called in this scope.
        // Omitted node 'N_sub_float'. Function already called in this scope.
        // Omitted node 'N_divtilesize_float'. Function already called in this scope.
        // Omitted node 'N_multtilesize_float'. Function already called in this scope.
        // Omitted node 'N_img_float'. Function already called in this scope.
        // Omitted node 'N_mult_float'. Function already called in this scope.
        // Omitted node 'N_sub_float'. Function already called in this scope.
        // Omitted node 'N_divtilesize_float'. Function already called in this scope.
        // Omitted node 'N_multtilesize_float'. Function already called in this scope.
        // Omitted node 'N_img_float'. Function already called in this scope.
        vd.positionWorld = hPositionWorld.xyz;

        return vd;
        // Omitted node 'geomprop_UV0'. Function already called in this scope.
        // Omitted node 'node_convert_1'. Function already called in this scope.
        // Omitted node 'node_rgbtohsv_12'. Function already called in this scope.
        // Omitted node 'geomprop_Nworld'. Function already called in this scope.
        // Omitted node 'geomprop_Tworld'. Function already called in this scope.
        // Omitted node 'node_tiledimage_float_26'. Function already called in this scope.
        // Omitted node 'node_tiledimage_float_7'. Function already called in this scope.
        // Omitted node 'node_tiledimage_float_24'. Function already called in this scope.
        // Omitted node 'node_tiledimage_float_10'. Function already called in this scope.
        // Omitted node 'node_tiledimage_float_22'. Function already called in this scope.
        // Omitted node 'node_tiledimage_vector3_27'. Function already called in this scope.
        // Omitted node 'node_multiply_25'. Function already called in this scope.
        // Omitted node 'node_multiply_20'. Function already called in this scope.
        // Omitted node 'node_multiply_9'. Function already called in this scope.
        // Omitted node 'node_multiply_23'. Function already called in this scope.
        // Omitted node 'node_max_1'. Function already called in this scope.
        // Omitted node 'node_normalmap_3'. Function already called in this scope.
        // Omitted node 'node_add_19'. Function already called in this scope.
        // Omitted node 'node_divide_21'. Function already called in this scope.
        // Omitted node 'node_subtract_18'. Function already called in this scope.
        // Omitted node 'node_multiply_15'. Function already called in this scope.
        // Omitted node 'node_multiply_1'. Function already called in this scope.
        // Omitted node 'node_multiply_14'. Function already called in this scope.
        // Omitted node 'node_combine3_color3_13'. Function already called in this scope.
        // Omitted node 'node_add_16'. Function already called in this scope.
        // Omitted node 'node_hsvtorgb_17'. Function already called in this scope.
        // Omitted node 'node_mix_6'. Function already called in this scope.
        // Omitted node 'node_multiply_5'. Function already called in this scope.
        // Omitted node 'node_mix_8'. Function already called in this scope.
        // Omitted node 'node_clamp_0'. Function already called in this scope.
        // Omitted node 'N_StandardSurface'. Function already called in this scope.
        // Omitted node 'M_BrickPattern'. Function already called in this scope.
    }

};
vertex VertexData VertexMain(
VertexInputs i_vs [[ stage_in ]], constant PrivateUniforms& u_prv[[ buffer(4) ]])
{
	GlobalContext ctx {i_vs.i_position, i_vs.i_texcoord_0, i_vs.i_normal, i_vs.i_tangent    , u_prv.u_worldMatrix
    , u_prv.u_viewProjectionMatrix
    , u_prv.u_worldInverseTransposeMatrix
    };
    VertexData out = ctx.VertexMain();
    out.pos.y = -out.pos.y;
    return out;
}

