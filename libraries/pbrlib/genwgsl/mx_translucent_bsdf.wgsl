// Generated from libraries/pbrlib/genglsl/mx_translucent_bsdf.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_closure_type.wgsl"

fn mx_translucent_bsdf(closureData_21: ClosureData, weight_7: f32, color_9: vec3f, N_24: vec3f, bsdf_8: ptr<function, BSDF>) {
    var closureData_22: ClosureData;
    var weight_8: f32;
    var color_10: vec3f;
    var N_25: vec3f;
    var V_8: vec3f;
    var L_4: vec3f;
    var NdotL_5: f32;
    var Li: vec3f;

    closureData_22 = closureData_21;
    weight_8 = weight_7;
    color_10 = color_9;
    N_25 = N_24;
    (*bsdf_8).throughput = vec3(0.0);
    if (weight_8 < 0.00000001) {
        {
            return;
        }
    }
    V_8 = closureData_22.V;
    L_4 = closureData_22.L;
    N_25 = -(N_25);
    if (closureData_22.closureType == 1i) {
        {
            NdotL_5 = clamp(dot(N_25, L_4), 0.0, 1.0);
            (*bsdf_8).response = (((color_10 * weight_8) * NdotL_5) * 0.31830987);
            return;
        }
    } else {
        if (closureData_22.closureType == 3i) {
            {
                Li = mx_environment_irradiance(N_25);
                (*bsdf_8).response = ((Li * color_10) * weight_8);
                return;
            }
        } else {
            return;
        }
    }
}
