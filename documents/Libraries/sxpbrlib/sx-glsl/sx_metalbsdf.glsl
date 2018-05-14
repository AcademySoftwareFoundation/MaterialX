#include "sxpbrlib/sx-glsl/lib/sx_bsdfs.glsl"

void sx_metalbsdf(vec3 L, vec3 V, vec3 ior_n, vec3 ior_k, float roughness, float anisotropy, vec3 normal, vec3 tangent, int distribution, out BSDF result)
{
    result.fr = vec3(0.0);
    result.ft = vec3(0.0);

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

    float VdotH = dot(V, H);
    vec3 F = sx_fresnel_conductor(VdotH, ior_n, ior_k);

    // Note: NdotL is cancelled out
    result.fr = F * D * G / (4 * NdotV);
}
