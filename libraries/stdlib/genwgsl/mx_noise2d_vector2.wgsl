// Generated from libraries/stdlib/genglsl/mx_noise2d_vector2.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_noise.wgsl"

fn mx_noise2d_vector2(amplitude_15: vec2f, pivot_7: f32, texcoord_29: vec2f, result_82: ptr<function, vec2f>) {
    var amplitude_16: vec2f;
    var pivot_8: f32;
    var texcoord_30: vec2f;
    var value_3: vec3f;

    amplitude_16 = amplitude_15;
    pivot_8 = pivot_7;
    texcoord_30 = texcoord_29;
    value_3 = mx_perlin_noise_vec3_2d(texcoord_30);
    (*result_82) = ((value_3.xy * amplitude_16) + vec2(pivot_8));
    return;
}
