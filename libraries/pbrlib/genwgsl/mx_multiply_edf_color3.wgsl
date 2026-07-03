// Generated from libraries/pbrlib/genglsl/mx_multiply_edf_color3.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_closure_type.wgsl"

fn mx_multiply_edf_color3(closureData_21: ClosureData, in1_8: vec3f, in2_8: vec3f, result_82: ptr<function, vec3f>) {
    var closureData_22: ClosureData;
    var in1_9: vec3f;
    var in2_9: vec3f;

    closureData_22 = closureData_21;
    in1_9 = in1_8;
    in2_9 = in2_8;
    (*result_82) = (in1_9 * in2_9);
    return;
}
