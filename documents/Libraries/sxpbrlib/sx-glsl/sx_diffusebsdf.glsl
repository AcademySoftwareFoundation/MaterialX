#include "sxpbrlib/sx-glsl/lib/sx_bsdfs.glsl"

void sx_diffusebsdf(vec3 L, vec3 V, vec3 reflectance, float roughness, vec3 normal, out BSDF result)
{
    result = BSDF(vec3(0.0), vec3(0.0));
    float NdotL = dot(L, normal);
    if (NdotL <= 0.0)
        return;

    result.fr = reflectance * NdotL * M_PI_INV;
    if (roughness > 0.0)
    {
        result.fr *= sx_orennayar(L, V, normal, NdotL, roughness);
    }
}
