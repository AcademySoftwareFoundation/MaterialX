#include "pbrlib/genglsl/lib/mx_bsdfs.glsl"

void mx_burley_diffuse_brdf_reflection(vec3 L, vec3 V, float weight, vec3 color, float roughness, vec3 normal, out BSDF result)
{
    float NdotL = dot(L, normal);
    if (NdotL <= 0.0 || weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    result = color * weight * NdotL * M_PI_INV;
    result *= mx_burley_diffuse(L, V, normal, NdotL, roughness);
    return;
}

void mx_burley_diffuse_brdf_indirect(vec3 V, float weight, vec3 color, float roughness, vec3 normal, out vec3 result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = vec3(0.0);
        return;
    }

    vec3 Li = mx_environment_irradiance(normal) *
              mx_burley_directional_albedo(V, normal, roughness);
    result = Li * color * weight;
}
