// Generated from libraries/pbrlib/genglsl/mx_sheen_bsdf.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_closure_type.wgsl"
#include "lib/mx_microfacet_sheen.wgsl"

fn mx_sheen_bsdf(closureData_21: ClosureData, weight_7: f32, color_9: vec3f, roughness_29: f32, N_24: vec3f, mode: i32, bsdf_8: ptr<function, BSDF>) {
    var closureData_22: ClosureData;
    var weight_8: f32;
    var color_10: vec3f;
    var roughness_30: f32;
    var N_25: vec3f;
    var V_8: vec3f;
    var L_4: vec3f;
    var NdotV_24: f32;
    var dirAlbedo: f32;
    var H_2: vec3f;
    var NdotL_5: f32;
    var NdotH_2: f32;
    var fr: vec3f;
    var fr_1: vec3f;
    var dirAlbedo_1: f32;
    var Li: vec3f;

    closureData_22 = closureData_21;
    weight_8 = weight_7;
    color_10 = color_9;
    roughness_30 = roughness_29;
    N_25 = N_24;
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
            if (mode == 0i) {
                {
                    H_2 = normalize((L_4 + V_8));
                    NdotL_5 = clamp(dot(N_25, L_4), 0.00000001, 1.0);
                    NdotH_2 = clamp(dot(N_25, H_2), 0.00000001, 1.0);
                    fr = (color_10 * (mx_imageworks_sheen_brdf(NdotL_5, NdotV_24, NdotH_2, roughness_30)));
                    dirAlbedo = (mx_imageworks_sheen_dir_albedo(NdotV_24, roughness_30));
                    (*bsdf_8).response = (((fr * NdotL_5) * closureData_22.occlusion) * weight_8);
                }
            } else {
                {
                    roughness_30 = clamp(roughness_30, 0.01, 1.0);
                    fr_1 = (color_10 * (mx_zeltner_sheen_brdf(L_4, V_8, N_25, NdotV_24, roughness_30)));
                    dirAlbedo = (mx_zeltner_sheen_dir_albedo(NdotV_24, roughness_30));
                    (*bsdf_8).response = (((dirAlbedo * fr_1) * closureData_22.occlusion) * weight_8);
                }
            }
            (*bsdf_8).throughput = vec3((1.0 - (dirAlbedo * weight_8)));
            return;
        }
    } else {
        if (closureData_22.closureType == 3i) {
            {
                if (mode == 0i) {
                    {
                        dirAlbedo_1 = (mx_imageworks_sheen_dir_albedo(NdotV_24, roughness_30));
                    }
                } else {
                    {
                        roughness_30 = clamp(roughness_30, 0.01, 1.0);
                        dirAlbedo_1 = (mx_zeltner_sheen_dir_albedo(NdotV_24, roughness_30));
                    }
                }
                Li = mx_environment_irradiance(N_25);
                (*bsdf_8).response = (((Li * color_10) * dirAlbedo_1) * weight_8);
                (*bsdf_8).throughput = vec3((1.0 - (dirAlbedo_1 * weight_8)));
                return;
            }
        } else {
            return;
        }
    }
}
