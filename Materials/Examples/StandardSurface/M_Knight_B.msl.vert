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
    vec3 i_normal [[attribute(1)]];
    vec3 i_tangent [[attribute(2)]];
    vec2 i_texcoord_0 [[attribute(3)]];
};
struct VertexData
{
    float4 pos [[position]];
    vec3 normalWorld;
    vec3 tangentWorld;
    vec2 texcoord_0;
    vec3 positionWorld;
};

struct GlobalContext
{
    GlobalContext(
    vec3 i_position
,     vec3 i_normal
,     vec3 i_tangent
,     vec2 i_texcoord_0
    ,     mat4 u_worldMatrix

    ,     mat4 u_viewProjectionMatrix

    ,     mat4 u_worldInverseTransposeMatrix

    ) : 
    i_position(i_position)
,     i_normal(i_normal)
,     i_tangent(i_tangent)
,     i_texcoord_0(i_texcoord_0)
    ,     u_worldMatrix(u_worldMatrix)

    ,     u_viewProjectionMatrix(u_viewProjectionMatrix)

    ,     u_worldInverseTransposeMatrix(u_worldInverseTransposeMatrix)

    {}
    vec3 i_position;

    vec3 i_normal;

    vec3 i_tangent;

    vec2 i_texcoord_0;
    
    mat4 u_worldMatrix;

    
    mat4 u_viewProjectionMatrix;

    
    mat4 u_worldInverseTransposeMatrix;

    VertexData VertexMain()
    {
        VertexData vd;
        float4 hPositionWorld = u_worldMatrix * float4(i_position, 1.0);
        vd.pos = u_viewProjectionMatrix * hPositionWorld;
        vd.normalWorld = normalize((u_worldInverseTransposeMatrix * vec4(i_normal, 0.0)).xyz);
        vd.tangentWorld = normalize((u_worldMatrix * vec4(i_tangent, 0.0)).xyz);
        vd.texcoord_0 = i_texcoord_0;
        vd.positionWorld = hPositionWorld.xyz;

        return vd;
        // Omitted node 'geomprop_Nworld'. Function already called in this scope.
        // Omitted node 'geomprop_Tworld'. Function already called in this scope.
        // Omitted node 'geomprop_UV0'. Function already called in this scope.
        // Omitted node 'diffuse4'. Function already called in this scope.
        // Omitted node 'roughness4'. Function already called in this scope.
        // Omitted node 'normal4'. Function already called in this scope.
        // Omitted node 'diffuse4_out_cm'. Function already called in this scope.
        // Omitted node 'mtlxnormalmap6'. Function already called in this scope.
        // Omitted node 'Knight_B'. Function already called in this scope.
        // Omitted node 'M_Knight_B'. Function already called in this scope.
    }

};
vertex VertexData VertexMain(
VertexInputs i_vs [[ stage_in ]], constant PrivateUniforms& u_prv[[ buffer(4) ]])
{
	GlobalContext ctx {i_vs.i_position, i_vs.i_normal, i_vs.i_tangent, i_vs.i_texcoord_0    , u_prv.u_worldMatrix
    , u_prv.u_viewProjectionMatrix
    , u_prv.u_worldInverseTransposeMatrix
    };
    VertexData out = ctx.VertexMain();
    out.pos.y = -out.pos.y;
    return out;
}

