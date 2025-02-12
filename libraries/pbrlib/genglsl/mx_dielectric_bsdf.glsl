#include "lib/mx_closure_type.glsl"
#include "lib/mx_microfacet_specular.glsl"

void mx_dielectric_bsdf(ClosureData closureData, float weight, vec3 tint, float ior, vec2 roughness, float thinfilm_thickness, float thinfilm_ior, vec3 N, vec3 X, int distribution, int scatter_mode, inout BSDF bsdf)
{
    if (weight < M_FLOAT_EPS)
    {
        return;
    }
    if (closureData.closureType != CLOSURE_TYPE_TRANSMISSION && scatter_mode == 1)
    {
        return;
    }

    vec3 V = closureData.V;
    vec3 L = closureData.L;

    N = mx_forward_facing_normal(N, V);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    FresnelData fd = mx_init_fresnel_dielectric(ior, thinfilm_thickness, thinfilm_ior);
    float F0 = mx_ior_to_f0(ior);

    vec2 safeAlpha = clamp(roughness, M_FLOAT_EPS, 1.0);
    float avgAlpha = mx_average_alpha(safeAlpha);
    vec3 safeTint = max(tint, 0.0);

    if (closureData.closureType == CLOSURE_TYPE_REFLECTION)
    {
        X = normalize(X - dot(X, N) * N);
        vec3 Y = cross(N, X);
        vec3 H = normalize(L + V);

        float NdotL = clamp(dot(N, L), M_FLOAT_EPS, 1.0);
        float VdotH = clamp(dot(V, H), M_FLOAT_EPS, 1.0);

        vec3 Ht = vec3(dot(H, X), dot(H, Y), dot(H, N));

        vec3 F = mx_compute_fresnel(VdotH, fd);
        float D = mx_ggx_NDF(Ht, safeAlpha);
        float G = mx_ggx_smith_G2(NdotL, NdotV, avgAlpha);

        vec3 comp = mx_ggx_energy_compensation(NdotV, avgAlpha, F);
        vec3 dirAlbedo = mx_ggx_dir_albedo(NdotV, avgAlpha, F0, 1.0) * comp;
        bsdf.throughput = 1.0 - dirAlbedo * weight;

        bsdf.response = D * F * G * comp * safeTint * closureData.occlusion * weight / (4.0 * NdotV);
    }
    else if (closureData.closureType == CLOSURE_TYPE_TRANSMISSION)
    {
        vec3 F = mx_compute_fresnel(NdotV, fd);

        vec3 comp = mx_ggx_energy_compensation(NdotV, avgAlpha, F);
        vec3 dirAlbedo = mx_ggx_dir_albedo(NdotV, avgAlpha, F0, 1.0) * comp;
        bsdf.throughput = 1.0 - dirAlbedo * weight;

        if (scatter_mode != 0)
        {
            bsdf.response = mx_surface_transmission(N, V, X, safeAlpha, distribution, fd, safeTint) * weight;
        }
    }
    else if (closureData.closureType == CLOSURE_TYPE_INDIRECT)
    {
        vec3 F = mx_compute_fresnel(NdotV, fd);

        vec3 comp = mx_ggx_energy_compensation(NdotV, avgAlpha, F);
        vec3 dirAlbedo = mx_ggx_dir_albedo(NdotV, avgAlpha, F0, 1.0) * comp;
        bsdf.throughput = 1.0 - dirAlbedo * weight;

        vec3 Li = mx_environment_radiance(N, V, X, safeAlpha, distribution, fd);
        bsdf.response = Li * safeTint * comp * weight;
    }
}
