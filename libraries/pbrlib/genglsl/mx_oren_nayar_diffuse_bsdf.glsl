#include "lib/mx_microfacet_diffuse.glsl"

void mx_oren_nayar_diffuse_bsdf_reflection(vec3 L, vec3 V, vec3 P, float occlusion, float weight, vec3 color, float roughness, vec3 normal, inout BSDF bsdf)
{
    bsdf.throughput = vec3(0.0);

    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    normal = mx_forward_facing_normal(normal, V);

    float NdotL = clamp(dot(normal, L), M_FLOAT_EPS, 1.0);

    bsdf.response = color * occlusion * weight * NdotL * M_PI_INV;
    if (roughness > 0.0)
    {
        bsdf.response *= mx_oren_nayar_diffuse(L, V, normal, NdotL, roughness);
    }
}

void mx_oren_nayar_diffuse_bsdf_indirect(vec3 V, float weight, vec3 color, float roughness, vec3 normal, inout BSDF bsdf)
{
    bsdf.throughput = vec3(0.0);

    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    normal = mx_forward_facing_normal(normal, V);

    vec3 Li = mx_environment_irradiance(normal);
    bsdf.response = Li * color * weight;
}
