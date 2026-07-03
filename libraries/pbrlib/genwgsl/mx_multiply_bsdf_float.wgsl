// Generated from libraries/pbrlib/genglsl/mx_multiply_bsdf_float.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_closure_type.wgsl"

fn mx_multiply_bsdf_float(closureData_21: ClosureData, in1_8: BSDF, in2_8: f32, result_82: ptr<function, BSDF>) {
    var closureData_22: ClosureData;
    var in1_9: BSDF;
    var in2_9: f32;
    var weight_8: f32;

    closureData_22 = closureData_21;
    in1_9 = in1_8;
    in2_9 = in2_8;
    weight_8 = clamp(in2_9, 0.0, 1.0);
    (*result_82).response = (in1_9.response * weight_8);
    (*result_82).throughput = in1_9.throughput;
    return;
}
