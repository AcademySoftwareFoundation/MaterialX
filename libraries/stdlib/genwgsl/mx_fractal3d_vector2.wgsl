// Generated from libraries/stdlib/genglsl/mx_fractal3d_vector2.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_noise.wgsl"

fn mx_fractal3d_vector2(amplitude_15: vec2f, octaves_15: i32, lacunarity_15: f32, diminish_15: f32, position_13: vec3f, result_82: ptr<function, vec2f>) {
    var amplitude_16: vec2f;
    var octaves_16: i32;
    var lacunarity_16: f32;
    var diminish_16: f32;
    var position_14: vec3f;
    var value_3: vec2f;

    amplitude_16 = amplitude_15;
    octaves_16 = octaves_15;
    lacunarity_16 = lacunarity_15;
    diminish_16 = diminish_15;
    position_14 = position_13;
    value_3 = (mx_fractal3d_noise_vec2(position_14, octaves_16, lacunarity_16, diminish_16));
    (*result_82) = (value_3 * amplitude_16);
    return;
}
