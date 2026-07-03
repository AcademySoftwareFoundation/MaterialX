// Generated from libraries/stdlib/genglsl/mx_noise3d_vector3.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_noise.wgsl"

fn mx_noise3d_vector3(amplitude_15: vec3f, pivot_7: f32, position_13: vec3f, result_82: ptr<function, vec3f>) {
    var amplitude_16: vec3f;
    var pivot_8: f32;
    var position_14: vec3f;
    var value_3: vec3f;

    amplitude_16 = amplitude_15;
    pivot_8 = pivot_7;
    position_14 = position_13;
    value_3 = mx_perlin_noise_vec3_3d(position_14);
    (*result_82) = ((value_3 * amplitude_16) + vec3(pivot_8));
    return;
}
