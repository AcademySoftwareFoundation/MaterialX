// Generated from libraries/stdlib/genglsl/mx_cellnoise3d_float.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_noise.wgsl"

fn mx_cellnoise3d_float(position_13: vec3f, result_82: ptr<function, f32>) {
    var position_14: vec3f;

    position_14 = position_13;
    (*result_82) = mx_cell_noise_float_vec3(position_14);
    return;
}
