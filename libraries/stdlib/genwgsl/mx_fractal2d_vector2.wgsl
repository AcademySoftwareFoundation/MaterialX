// Generated from libraries/stdlib/genglsl/mx_fractal2d_vector2.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_noise.wgsl"

fn mx_fractal2d_vector2(amplitude_15: vec2f, octaves_15: i32, lacunarity_15: f32, diminish_15: f32, texcoord_29: vec2f, result_82: ptr<function, vec2f>) {
    var amplitude_16: vec2f;
    var octaves_16: i32;
    var lacunarity_16: f32;
    var diminish_16: f32;
    var texcoord_30: vec2f;
    var value_3: vec2f;

    amplitude_16 = amplitude_15;
    octaves_16 = octaves_15;
    lacunarity_16 = lacunarity_15;
    diminish_16 = diminish_15;
    texcoord_30 = texcoord_29;
    value_3 = (mx_fractal2d_noise_vec2(texcoord_30, octaves_16, lacunarity_16, diminish_16));
    (*result_82) = (value_3 * amplitude_16);
    return;
}
