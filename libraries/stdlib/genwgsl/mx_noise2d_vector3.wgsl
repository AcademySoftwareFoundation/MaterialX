// Generated from libraries/stdlib/genglsl/mx_noise2d_vector3.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_noise.wgsl"

fn mx_noise2d_vector3(amplitude_15: vec3f, pivot_7: f32, texcoord_29: vec2f, result_82: ptr<function, vec3f>) {
    var amplitude_16: vec3f;
    var pivot_8: f32;
    var texcoord_30: vec2f;
    var value_3: vec3f;

    amplitude_16 = amplitude_15;
    pivot_8 = pivot_7;
    texcoord_30 = texcoord_29;
    value_3 = mx_perlin_noise_vec3_2d(texcoord_30);
    (*result_82) = ((value_3 * amplitude_16) + vec3(pivot_8));
    return;
}
