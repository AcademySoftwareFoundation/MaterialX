// Generated from libraries/pbrlib/genglsl/mx_layer_bsdf.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_closure_type.wgsl"

fn mx_layer_bsdf(closureData_21: ClosureData, top_1: BSDF, base_2: BSDF, result_82: ptr<function, BSDF>) {
    var closureData_22: ClosureData;
    var top_2: BSDF;
    var base_3: BSDF;

    closureData_22 = closureData_21;
    top_2 = top_1;
    base_3 = base_2;
    (*result_82).response = (top_2.response + (base_3.response * top_2.throughput));
    (*result_82).throughput = (top_2.throughput * base_3.throughput);
    return;
}
