// Generated from libraries/pbrlib/genglsl/mx_layer_vdf.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_closure_type.wgsl"

fn mx_layer_vdf(closureData_21: ClosureData, top_1: BSDF, base_2: VDF, result_82: ptr<function, BSDF>) {
    var closureData_22: ClosureData;
    var top_2: BSDF;
    var base_3: VDF;

    closureData_22 = closureData_21;
    top_2 = top_1;
    base_3 = base_2;
    (*result_82).response = (top_2.response * base_3.throughput);
    (*result_82).throughput = (top_2.throughput * base_3.throughput);
    return;
}
