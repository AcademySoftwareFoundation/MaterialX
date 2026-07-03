// Generated from libraries/pbrlib/genglsl/mx_generalized_schlick_edf.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_closure_type.wgsl"
#include "lib/mx_microfacet.wgsl"

fn mx_generalized_schlick_edf(closureData_21: ClosureData, color0_1: vec3f, color90_1: vec3f, exponent_4: f32, base_2: vec3f, result_82: ptr<function, vec3f>) {
    var closureData_22: ClosureData;
    var color0_2: vec3f;
    var color90_2: vec3f;
    var exponent_5: f32;
    var base_3: vec3f;
    var N_25: vec3f;
    var NdotV_24: f32;
    var f_1: vec3f;

    closureData_22 = closureData_21;
    color0_2 = color0_1;
    color90_2 = color90_1;
    exponent_5 = exponent_4;
    base_3 = base_2;
    if (closureData_22.closureType == 4i) {
        {
            N_25 = (mx_forward_facing_normal(closureData_22.N, closureData_22.V));
            NdotV_24 = clamp(dot(N_25, closureData_22.V), 0.00000001, 1.0);
            f_1 = (mx_fresnel_schlick_vec3_exp(NdotV_24, color0_2, color90_2, exponent_5));
            (*result_82) = (base_3 * f_1);
            return;
        }
    } else {
        return;
    }
}
