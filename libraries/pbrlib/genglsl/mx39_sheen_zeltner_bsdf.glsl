#include "lib/mx39_microfacet_sheen.glsl"

void mx39_sheen_zeltner_bsdf_reflection(vec3 L, vec3 V, vec3 P, float occlusion, float weight, vec3 color, float roughness, vec3 N, inout BSDF bsdf)
{
    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    N = mx_forward_facing_normal(N, V);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    roughness = clamp(roughness, 0.01, 1.0); // Clamp to range of original impl.

    vec3 fr = color * mx39_zeltner_sheen_brdf(L, V, N, NdotV, roughness);
    float dirAlbedo = mx39_zeltner_sheen_dir_albedo(NdotV, roughness);
    bsdf.throughput = vec3(1.0 - dirAlbedo * weight);
    bsdf.response = dirAlbedo * fr * occlusion * weight;
}

void mx39_sheen_zeltner_bsdf_indirect(vec3 V, float weight, vec3 color, float roughness, vec3 N, inout BSDF bsdf)
{
    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    N = mx_forward_facing_normal(N, V);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    float dirAlbedo;
    roughness = clamp(roughness, 0.01, 1.0); // Clamp to range of original impl.
    dirAlbedo = mx39_zeltner_sheen_dir_albedo(NdotV, roughness);

    vec3 Li = mx_environment_irradiance(N);
    bsdf.throughput = vec3(1.0 - dirAlbedo * weight);
    bsdf.response = Li * color * dirAlbedo * weight;
}
