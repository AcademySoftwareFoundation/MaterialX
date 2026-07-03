// Generated from libraries/stdlib/genglsl/mx_worleynoise2d_float.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_noise.wgsl"

fn mx_worleynoise2d_float(texcoord_29: vec2f, jitter_15: f32, style_11: i32, result_82: ptr<function, f32>) {
    var texcoord_30: vec2f;
    var jitter_16: f32;
    var style_12: i32;

    texcoord_30 = texcoord_29;
    jitter_16 = jitter_15;
    style_12 = style_11;
    (*result_82) = (mx_worley_noise_float_2d(texcoord_30, jitter_16, style_12, 0i));
    return;
}
