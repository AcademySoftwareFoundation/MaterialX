// Generated from libraries/pbrlib/genglsl/mx_multiply_bsdf_color3.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_closure_type.wgsl"

fn mx_multiply_bsdf_color3(closureData_21: ClosureData, in1_8: BSDF, in2_8: vec3f, result_82: ptr<function, BSDF>) {
    var closureData_22: ClosureData;
    var in1_9: BSDF;
    var in2_9: vec3f;
    var tint_2: vec3f;

    closureData_22 = closureData_21;
    in1_9 = in1_8;
    in2_9 = in2_8;
    tint_2 = clamp(in2_9, vec3(0.0), vec3(1.0));
    (*result_82).response = (in1_9.response * tint_2);
    (*result_82).throughput = in1_9.throughput;
    return;
}
