// Generated from libraries/pbrlib/genglsl/mx_uniform_edf.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_closure_type.wgsl"

fn mx_uniform_edf(closureData_21: ClosureData, color_9: vec3f, result_82: ptr<function, vec3f>) {
    var closureData_22: ClosureData;
    var color_10: vec3f;

    closureData_22 = closureData_21;
    color_10 = color_9;
    if (closureData_22.closureType == 4i) {
        {
            (*result_82) = color_10;
            return;
        }
    } else {
        return;
    }
}
