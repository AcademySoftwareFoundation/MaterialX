#include "sxpbrlib/sx-glsl/lib/sx_bsdfs.glsl"

void sx_diffusebrdf_reflection(vec3 L, vec3 V, float weight, vec3 color, float roughness, vec3 normal, out BSDF result)
{
    float NdotL = dot(L, normal);
    if (NdotL <= 0.0 || weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    result = color * weight * NdotL * M_PI_INV;
    if (roughness > 0.0)
    {
        result *= sx_orennayar(L, V, normal, NdotL, roughness);
    }
}

void sx_diffusebrdf_indirect(vec3 V, float weight, vec3 color, float roughness, vec3 normal, out vec3 result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = vec3(0.0);
        return;
    }

    vec3 Li = sx_environment_irradiance(normal);
    result = Li * color * weight;
}
