// Generated from libraries/pbrlib/genglsl/mx_mix_bsdf.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_closure_type.wgsl"

fn mx_mix_bsdf(closureData_21: ClosureData, fg_9: BSDF, bg_9: BSDF, mixValue_1: f32, result_82: ptr<function, BSDF>) {
    var closureData_22: ClosureData;
    var fg_10: BSDF;
    var bg_10: BSDF;
    var mixValue_2: f32;

    closureData_22 = closureData_21;
    fg_10 = fg_9;
    bg_10 = bg_9;
    mixValue_2 = mixValue_1;
    (*result_82).response = mix(bg_10.response, fg_10.response, vec3(mixValue_2));
    (*result_82).throughput = mix(bg_10.throughput, fg_10.throughput, vec3(mixValue_2));
    return;
}
