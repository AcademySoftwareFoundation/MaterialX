// Platform-specific shadow map lookup.
// Requires (provided by the renderer's shader template):
//   shadowMap        : texture_2d<f32>  — variance shadow map
//   shadowMapSampler : sampler          — sampler for shadowMap

fn mx_shadow_occlusion(shadow_matrix: mat4x4f, world_position: vec3f) -> f32 {
    let shadowCoord4 = (shadow_matrix * vec4f(world_position, 1.0));
    let shadowCoord = shadowCoord4.xyz / shadowCoord4.w * 0.5 + 0.5;
    let shadowMoments = textureSample(shadowMap, shadowMapSampler, shadowCoord.xy).xy;
    return mx_variance_shadow_occlusion(shadowMoments, shadowCoord.z);
}
