#include "lib/mx_microfacet_diffuse.glsl"

void mx_oren_nayar_diffuse_bsdf_reflection(vec3 L, vec3 V, vec3 P, float occlusion, float weight, vec3 color, float roughness, vec3 normal, bool energy_compensation, inout BSDF bsdf)
{
    bsdf.throughput = vec3(0.0);

    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    normal = mx_forward_facing_normal(normal, V);

    float NdotV = clamp(dot(normal, V), M_FLOAT_EPS, 1.0);
    float NdotL = clamp(dot(normal, L), M_FLOAT_EPS, 1.0);
    float LdotV = clamp(dot(L, V), M_FLOAT_EPS, 1.0);

    vec3 diffuse;
    if (energy_compensation)
    {
        diffuse = mx_energy_compensated_oren_nayar_diffuse(NdotV, NdotL, LdotV, roughness, color);
    }
    else
    {
        diffuse = mx_oren_nayar_diffuse(NdotV, NdotL, LdotV, roughness) * color;
    }

    bsdf.response = diffuse * occlusion * weight * NdotL * M_PI_INV;
}

void mx_oren_nayar_diffuse_bsdf_indirect(vec3 V, float weight, vec3 color, float roughness, vec3 normal, bool energy_compensation, inout BSDF bsdf)
{
    bsdf.throughput = vec3(0.0);

    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    normal = mx_forward_facing_normal(normal, V);

    float NdotV = clamp(dot(normal, V), M_FLOAT_EPS, 1.0);

    float dirAlbedo;
    if (energy_compensation)
    {
        dirAlbedo = mx_oren_nayar_fujii_diffuse_dir_albedo(NdotV, roughness) /
                    mx_oren_nayar_fujii_diffuse_avg_albedo(roughness);
    }
    else
    {
        dirAlbedo = mx_oren_nayar_diffuse_dir_albedo(NdotV, roughness);
    }

    vec3 Li = mx_environment_irradiance(normal);
    bsdf.response = Li * dirAlbedo * color * weight;
}
