
float mx_shadow_occlusion(
    MetalTexture shadow_map_tex,
    mat4 shadow_matrix,
    vec3 world_position
)
{
    vec3 shadowCoord = mx_matrix_mul(shadow_matrix, vec4(world_position, 1.0)).xyz;
    shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5;
    vec2 shadowMoments = texture(shadow_map_tex, shadowCoord.xy).xy;
    return  mx_variance_shadow_occlusion(shadowMoments, shadowCoord.z);
}
