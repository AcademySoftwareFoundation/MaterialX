// Generated from libraries/stdlib/genglsl/mx_noise3d_vector4.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_noise.wgsl"

fn mx_noise3d_vector4(amplitude_15: vec4f, pivot_7: f32, position_13: vec3f, result_82: ptr<function, vec4f>) {
    var amplitude_16: vec4f;
    var pivot_8: f32;
    var position_14: vec3f;
    var xyz: vec3f;
    var w_1: f32;

    amplitude_16 = amplitude_15;
    pivot_8 = pivot_7;
    position_14 = position_13;
    xyz = mx_perlin_noise_vec3_3d(position_14);
    w_1 = (mx_perlin_noise_float_3d((position_14 + vec3f(19.0, 73.0, 29.0))));
    let _e39 = xyz;
    (*result_82) = ((vec4f(_e39.x, _e39.y, _e39.z, w_1) * amplitude_16) + vec4(pivot_8));
    return;
}
