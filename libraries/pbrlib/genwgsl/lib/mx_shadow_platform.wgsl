// Hand-written WGSL shadow helpers (see skip_transpile.txt).
// mx_shadow_occlusion uses split texture/sampler bindings for WebGPU.
// mx_compute_depth_moments takes window-space depth (no gl_FragCoord in WGSL).

fn mx_shadow_occlusion(
    shadow_map: texture_2d<f32>,
    shadow_sampler: sampler,
    shadow_matrix: mat4x4<f32>,
    world_position: vec3f,
) -> f32 {
    // WebGPU clip space: flip Y for texture V; NDC Z is already [0, 1]
    let shadowCoord4 = shadow_matrix * vec4f(world_position, 1.0);
    let shadowNdc = shadowCoord4.xyz / shadowCoord4.w;
    let shadowUv = vec2f(shadowNdc.x * 0.5 + 0.5, 0.5 - shadowNdc.y * 0.5);
    let shadowMoments = textureSample(shadow_map, shadow_sampler, shadowUv).xy;
    return mx_variance_shadow_occlusion(shadowMoments, shadowNdc.z);
}

fn mx_compute_depth_moments(depth: f32) -> vec2f {
    return vec2f(depth, mx_square_f32(depth));
}
