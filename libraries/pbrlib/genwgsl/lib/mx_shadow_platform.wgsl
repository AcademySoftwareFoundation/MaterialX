// Hand-written WGSL shadow helpers (see skip_transpile.txt).
// mx_shadow_occlusion uses split texture/sampler bindings for WebGPU.
// mx_compute_depth_moments mirrors genglsl/lib/mx_shadow_platform.glsl for future
// hwWriteDepthMoments support in WgslShaderGenerator (pass window-space depth).

fn mx_shadow_occlusion(
    shadow_map: texture_2d<f32>,
    shadow_sampler: sampler,
    shadow_matrix: mat4x4<f32>,
    world_position: vec3f,
) -> f32 {
    let shadowCoord4 = shadow_matrix * vec4f(world_position, 1.0);
    var shadowCoord = shadowCoord4.xyz / shadowCoord4.w;
    shadowCoord = shadowCoord * 0.5 + 0.5;
    let shadowMoments = textureSample(shadow_map, shadow_sampler, shadowCoord.xy).xy;
    return mx_variance_shadow_occlusion(shadowMoments, shadowCoord.z);
}

fn mx_compute_depth_moments(depth: f32) -> vec2f {
    return vec2f(depth, mx_square(depth));
}
