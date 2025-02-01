#include "lib/mx_microfacet_diffuse.glsl"

void mx_burley_diffuse_bsdf(ClosureData closureData, float weight, vec3 color, float roughness, vec3 normal, inout BSDF bsdf)
{
    vec3 V = closureData.V;
    vec3 L = closureData.L;
    float occlusion = closureData.occlusion;

    bsdf.throughput = vec3(0.0);

    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    normal = mx_forward_facing_normal(normal, V);

    float NdotV = clamp(dot(normal, V), M_FLOAT_EPS, 1.0);

    if (closureData.closureType == CLOSURE_TYPE_REFLECTION)
    {
        float NdotL = clamp(dot(normal, L), M_FLOAT_EPS, 1.0);
        float LdotH = clamp(dot(L, normalize(L + V)), M_FLOAT_EPS, 1.0);

        bsdf.response = color * occlusion * weight * NdotL * M_PI_INV;
        bsdf.response *= mx_burley_diffuse(NdotV, NdotL, LdotH, roughness);
    }
    else if (closureData.closureType == CLOSURE_TYPE_INDIRECT)
    {
        vec3 Li = mx_environment_irradiance(normal) *
                  mx_burley_diffuse_dir_albedo(NdotV, roughness);
        bsdf.response = Li * color * weight;
    }
}
