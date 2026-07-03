// Generated from libraries/stdlib/genglsl/mx_noise2d_vector4.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_noise.wgsl"

fn mx_noise2d_vector4(amplitude_15: vec4f, pivot_7: f32, texcoord_29: vec2f, result_82: ptr<function, vec4f>) {
    var amplitude_16: vec4f;
    var pivot_8: f32;
    var texcoord_30: vec2f;
    var xyz: vec3f;
    var w_1: f32;

    amplitude_16 = amplitude_15;
    pivot_8 = pivot_7;
    texcoord_30 = texcoord_29;
    xyz = mx_perlin_noise_vec3_2d(texcoord_30);
    w_1 = (mx_perlin_noise_float_2d((texcoord_30 + vec2f(19.0, 73.0))));
    let _e37 = xyz;
    (*result_82) = ((vec4f(_e37.x, _e37.y, _e37.z, w_1) * amplitude_16) + vec4(pivot_8));
    return;
}
