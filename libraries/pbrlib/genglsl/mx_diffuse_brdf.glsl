#include "pbrlib/genglsl/lib/mx_microfacet_diffuse.glsl"

void mx_diffuse_brdf_reflection(vec3 L, vec3 V, float weight, vec3 color, float roughness, vec3 normal, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    float NdotL = clamp(dot(normal, L), M_FLOAT_EPS, 1.0);

    result = color * weight * NdotL * M_PI_INV;
    if (roughness > 0.0)
    {
        result *= mx_oren_nayar_diffuse(L, V, normal, NdotL, roughness);
    }
}

void mx_diffuse_brdf_indirect(vec3 V, float weight, vec3 color, float roughness, vec3 normal, out vec3 result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    vec3 Li = mx_environment_irradiance(normal);
    result = Li * color * weight;
}
