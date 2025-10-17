
float mx_shadow_occlusion(
    $texSamplerSignature,
    mat4 shadow_matrix,
    vec3 world_position
)
{
    vec4 shadowCoord4 = mx_matrix_mul(shadow_matrix, vec4(world_position, 1.0));
    vec3 shadowCoord = shadowCoord4.xyz / shadowCoord4.w;
    shadowCoord = shadowCoord * 0.5 + 0.5;
    vec2 shadowMoments = texture($texSamplerSampler2D, shadowCoord.xy).xy;
    return  mx_variance_shadow_occlusion(shadowMoments, shadowCoord.z);
}
