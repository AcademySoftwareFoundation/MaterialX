#include "sxpbrlib/sx-glsl/lib/sx_bsdfs.glsl"
#include "sxpbrlib/sx-glsl/lib/sx_complexior.glsl"

void sx_conductorbrdf(vec3 L, vec3 V, float weight, vec3 reflectivity, vec3 edgecolor, vec2 roughness, vec3 normal, vec3 tangent, int distribution, out BSDF result)
{
    result = BSDF(0.0);
    if (weight < M_FLOAT_EPS)
        return;

    float NdotL = dot(normal,L);
    float NdotV = dot(normal,V);
    if (NdotL <= 0.0 || NdotV <= 0.0)
        return;

    vec3 bitangent = normalize(cross(normal, tangent));

    vec3 H = normalize(L + V);
    float NdotH = dot(normal, H);

    float D = sx_microfacet_ggx_NDF(tangent, bitangent, H, NdotH, roughness.x, roughness.y);
    float G = sx_microfacet_ggx_smith_G(NdotL, NdotV, roughness.x);

    vec3 ior_n, ior_k;
    sx_complexior(reflectivity, edgecolor, ior_n, ior_k);

    float VdotH = dot(V, H);
    vec3 F = sx_fresnel_conductor(VdotH, ior_n, ior_k);
    F *= weight;

    // Note: NdotL is cancelled out
    result = F * D * G / (4 * NdotV);
}

void sx_conductorbrdf_ibl(vec3 V, float weight, vec3 reflectivity, vec3 edgecolor, vec2 roughness, vec3 normal, vec3 tangent, int distribution, out vec3 result)
{
    result = vec3(0.0);
    if (weight < M_FLOAT_EPS)
        return;

    vec3 ior_n, ior_k;
    sx_complexior(reflectivity, edgecolor, ior_n, ior_k);

    vec3 Li = sx_environment_specular(normal, V, roughness.x);
    vec3 F = sx_fresnel_conductor(dot(normal, V), ior_n, ior_k);
    F *= weight;
    result = Li * F;
}
