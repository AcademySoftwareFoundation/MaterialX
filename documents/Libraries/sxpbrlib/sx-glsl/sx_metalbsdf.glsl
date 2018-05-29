#include "sxpbrlib/sx-glsl/lib/sx_bsdfs.glsl"
#include "sxpbrlib/sx-glsl/lib/sx_complexior.glsl"

void sx_metalbsdf(vec3 L, vec3 V, vec3 reflectivity, vec3 edgetint, float roughness, float anisotropy, vec3 normal, vec3 tangent, int distribution, out BSDF result)
{
    result = BSDF(0.0);

    float NdotL = dot(normal,L);
    float NdotV = dot(normal,V);
    if (NdotL <= 0.0 || NdotV <= 0.0)
        return;

    vec3 bitangent = normalize(cross(normal, tangent));

    vec3 H = normalize(L + V);
    float NdotH = dot(normal, H);

    float alpha = clamp(roughness*roughness, M_FLOAT_EPS, 1.0);
    float alphaX = alpha;
    float alphaY = alpha;
    if (anisotropy > 0.0)
    {
        anisotropy = clamp(anisotropy, 0.0, 0.98);
        float aspect = sqrt(1.0 - anisotropy);
        alphaX = min(alpha / aspect, 1.0);
        alphaY = alpha * aspect;
    }

    float D = sx_microfacet_ggx_NDF(tangent, bitangent, H, NdotH, alphaX, alphaY);
    float G = sx_microfacet_ggx_smith_G(NdotL, NdotV, alpha);

    vec3 ior_n, ior_k;
    sx_complexior(reflectivity, edgetint, ior_n, ior_k);

    float VdotH = dot(V, H);
    vec3 F = sx_fresnel_conductor(VdotH, ior_n, ior_k);

    // Note: NdotL is cancelled out
    result = F * D * G / (4 * NdotV);
}

void sx_metalbsdf_ibl(vec3 V, vec3 reflectivity, vec3 edgetint, float roughness, float anisotropy, vec3 normal, vec3 tangent, int distribution, out vec3 result)
{
    vec3 ior_n, ior_k;
    sx_complexior(reflectivity, edgetint, ior_n, ior_k);

    vec3 Li = sx_environment_specular(normal, V, roughness);
    vec3 F = sx_fresnel_conductor(dot(normal, V), ior_n, ior_k);
    result = Li * F;
}
