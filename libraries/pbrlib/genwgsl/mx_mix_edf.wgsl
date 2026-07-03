// Generated from libraries/pbrlib/genglsl/mx_mix_edf.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_closure_type.wgsl"

fn mx_mix_edf(closureData_21: ClosureData, fg_9: vec3f, bg_9: vec3f, mixValue_1: f32, result_82: ptr<function, vec3f>) {
    var closureData_22: ClosureData;
    var fg_10: vec3f;
    var bg_10: vec3f;
    var mixValue_2: f32;

    closureData_22 = closureData_21;
    fg_10 = fg_9;
    bg_10 = bg_9;
    mixValue_2 = mixValue_1;
    (*result_82) = mix(bg_10, fg_10, vec3(mixValue_2));
    return;
}
