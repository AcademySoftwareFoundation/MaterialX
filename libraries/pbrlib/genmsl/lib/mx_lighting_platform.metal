
float mx_shadow_occlusion(
    MetalTexture shadow_map_tex,
    float4x4 shadow_matrix,
    float3 world_position
)
{
    float4 shadowCoord4 = mx_matrix_mul(shadow_matrix, float4(world_position, 1.0));
    float3 shadowCoord = shadowCoord4.xyz / shadowCoord4.w;
    shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5;
    float2 shadowMoments = texture(shadow_map_tex, shadowCoord.xy).xy;
    return mx_variance_shadow_occlusion(shadowMoments, shadowCoord.z);
}
