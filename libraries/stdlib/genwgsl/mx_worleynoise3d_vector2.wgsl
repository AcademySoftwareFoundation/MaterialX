// Generated from libraries/stdlib/genglsl/mx_worleynoise3d_vector2.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_noise.wgsl"

fn mx_worleynoise3d_vector2(position_13: vec3f, jitter_15: f32, style_11: i32, result_82: ptr<function, vec2f>) {
    var position_14: vec3f;
    var jitter_16: f32;
    var style_12: i32;

    position_14 = position_13;
    jitter_16 = jitter_15;
    style_12 = style_11;
    (*result_82) = (mx_worley_noise_vec2_3d(position_14, jitter_16, style_12, 0i));
    return;
}
