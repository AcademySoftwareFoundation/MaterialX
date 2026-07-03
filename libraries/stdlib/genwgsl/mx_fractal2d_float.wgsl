// Generated from libraries/stdlib/genglsl/mx_fractal2d_float.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_noise.wgsl"

fn mx_fractal2d_float(amplitude_15: f32, octaves_15: i32, lacunarity_15: f32, diminish_15: f32, texcoord_29: vec2f, result_82: ptr<function, f32>) {
    var amplitude_16: f32;
    var octaves_16: i32;
    var lacunarity_16: f32;
    var diminish_16: f32;
    var texcoord_30: vec2f;
    var value_3: f32;

    amplitude_16 = amplitude_15;
    octaves_16 = octaves_15;
    lacunarity_16 = lacunarity_15;
    diminish_16 = diminish_15;
    texcoord_30 = texcoord_29;
    value_3 = (mx_fractal2d_noise_float(texcoord_30, octaves_16, lacunarity_16, diminish_16));
    (*result_82) = (value_3 * amplitude_16);
    return;
}
