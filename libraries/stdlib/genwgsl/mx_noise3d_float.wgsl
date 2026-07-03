// Generated from libraries/stdlib/genglsl/mx_noise3d_float.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_noise.wgsl"

fn mx_noise3d_float(amplitude_15: f32, pivot_7: f32, position_13: vec3f, result_82: ptr<function, f32>) {
    var amplitude_16: f32;
    var pivot_8: f32;
    var position_14: vec3f;
    var value_3: f32;

    amplitude_16 = amplitude_15;
    pivot_8 = pivot_7;
    position_14 = position_13;
    value_3 = mx_perlin_noise_float_3d(position_14);
    (*result_82) = ((value_3 * amplitude_16) + pivot_8);
    return;
}
