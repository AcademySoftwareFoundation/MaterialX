#include "pbrlib/genglsl/lib/mx_bsdfs.glsl"

void mx_dielectric_brdf_reflection(vec3 L, vec3 V, float weight, vec3 tint, float ior, roughnessinfo roughness, vec3 N, vec3 X, int distribution, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    float NdotL = dot(N,L);
    float NdotV = dot(N,V);
    if (NdotL <= 0.0 || NdotV <= 0.0)
    {
        result = base;
        return;
    }

    vec3 Y = normalize(cross(N, X));

    vec3 H = normalize(L + V);
    float NdotH = dot(N, H);

    float D = mx_microfacet_ggx_NDF(X, Y, H, NdotH, roughness.alphaX, roughness.alphaY);
    float G = mx_microfacet_ggx_smith_G(NdotL, NdotV, roughness.alpha);

    float VdotH = dot(V, H);
    float F = mx_fresnel_schlick(VdotH, ior);
    F *= weight;

    // Note: NdotL is cancelled out
    result = tint * D * G * F / (4 * NdotV) // Top layer reflection
           + base * (1.0 - F);              // Base layer reflection attenuated by top fresnel
}

void mx_dielectric_brdf_transmission(vec3 V, float weight, vec3 tint, float ior, roughnessinfo roughness, vec3 N, vec3 X, int distribution, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    // Dielectric BRDF has no transmission but we must
    // attenuate the base layer transmission by the
    // inverse of top layer reflectance.

    // Abs here to allow transparency through backfaces
    float NdotV = abs(dot(N,V));
    float F = mx_fresnel_schlick(NdotV, ior);
    F *= weight;

    result = base * (1.0 - F); // Base layer transmission attenuated by top fresnel
}

void mx_dielectric_brdf_indirect(vec3 V, float weight, vec3 tint, float ior, roughnessinfo roughness, vec3 N, vec3 X, int distribution, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    vec3 Li = mx_environment_radiance(N, V, X, roughness, distribution);

    float NdotV = dot(N,V);
    float F = mx_fresnel_schlick_roughness(NdotV, ior, roughness.alpha);
    F *= weight;

    result = Li * tint * F     // Top layer reflection
           + base * (1.0 - F); // Base layer reflection attenuated by top fresnel
}
