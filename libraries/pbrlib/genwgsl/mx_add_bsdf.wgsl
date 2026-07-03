// Generated from libraries/pbrlib/genglsl/mx_add_bsdf.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_closure_type.wgsl"

fn mx_add_bsdf(closureData_21: ClosureData, in1_8: BSDF, in2_8: BSDF, result_82: ptr<function, BSDF>) {
    var closureData_22: ClosureData;
    var in1_9: BSDF;
    var in2_9: BSDF;

    closureData_22 = closureData_21;
    in1_9 = in1_8;
    in2_9 = in2_8;
    (*result_82).response = (in1_9.response + in2_9.response);
    (*result_82).throughput = max(((in1_9.throughput + in2_9.throughput) - vec3(1.0)), vec3(0.0));
    return;
}
