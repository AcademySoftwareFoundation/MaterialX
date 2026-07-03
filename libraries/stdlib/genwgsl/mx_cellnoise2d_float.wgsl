// Generated from libraries/stdlib/genglsl/mx_cellnoise2d_float.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_noise.wgsl"

fn mx_cellnoise2d_float(texcoord_29: vec2f, result_82: ptr<function, f32>) {
    var texcoord_30: vec2f;

    texcoord_30 = texcoord_29;
    (*result_82) = mx_cell_noise_float_vec2(texcoord_30);
    return;
}
