// Generated from libraries/pbrlib/genglsl/mx_oren_nayar_diffuse_bsdf.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_closure_type.wgsl"
#include "lib/mx_microfacet_diffuse.wgsl"

fn mx_oren_nayar_diffuse_bsdf(closureData_21: ClosureData, weight_7: f32, color_9: vec3f, roughness_29: f32, N_24: vec3f, energy_compensation: bool, bsdf_8: ptr<function, BSDF>) {
    var closureData_22: ClosureData;
    var weight_8: f32;
    var color_10: vec3f;
    var roughness_30: f32;
    var N_25: vec3f;
    var V_8: vec3f;
    var L_4: vec3f;
    var NdotV_24: f32;
    var NdotL_5: f32;
    var LdotV_2: f32;
    var local: vec3f;
    var diffuse: vec3f;
    var local_1: vec3f;
    var diffuse_1: vec3f;
    var Li: vec3f;

    closureData_22 = closureData_21;
    weight_8 = weight_7;
    color_10 = color_9;
    roughness_30 = roughness_29;
    N_25 = N_24;
    (*bsdf_8).throughput = vec3(0.0);
    if (weight_8 < 0.00000001) {
        {
            return;
        }
    }
    V_8 = closureData_22.V;
    L_4 = closureData_22.L;
    N_25 = (mx_forward_facing_normal(N_25, V_8));
    NdotV_24 = clamp(dot(N_25, V_8), 0.00000001, 1.0);
    if (closureData_22.closureType == 1i) {
        {
            NdotL_5 = clamp(dot(N_25, L_4), 0.00000001, 1.0);
            LdotV_2 = clamp(dot(L_4, V_8), 0.00000001, 1.0);
            if energy_compensation {
                local = (mx_oren_nayar_compensated_diffuse(NdotV_24, NdotL_5, LdotV_2, roughness_30, color_10));
            } else {
                local = ((mx_oren_nayar_diffuse(NdotV_24, NdotL_5, LdotV_2, roughness_30)) * color_10);
            }
            diffuse = local;
            (*bsdf_8).response = ((((diffuse * closureData_22.occlusion) * weight_8) * NdotL_5) * 0.31830987);
            return;
        }
    } else {
        if (closureData_22.closureType == 3i) {
            {
                if energy_compensation {
                    local_1 = (mx_oren_nayar_compensated_diffuse_dir_albedo(NdotV_24, roughness_30, color_10));
                } else {
                    local_1 = ((mx_oren_nayar_diffuse_dir_albedo(NdotV_24, roughness_30)) * color_10);
                }
                diffuse_1 = local_1;
                Li = mx_environment_irradiance(N_25);
                (*bsdf_8).response = ((Li * diffuse_1) * weight_8);
                return;
            }
        } else {
            return;
        }
    }
}
