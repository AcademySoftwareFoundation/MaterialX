#include "sxpbrlib/sx-glsl/lib/sx_bsdfs.glsl"

void sx_coatingbsdf(vec3 L, vec3 V, vec3 reflectance, float ior, float roughness, float anisotropy, vec3 normal, vec3 tangent, int distribution, BSDF base, out BSDF result)
{
    result = base;

    normal = sx_front_facing(normal);
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
    float F = sx_fresnel_schlick(VdotH, ior);

    // Note: NdotL is cancelled out
    result.fr = reflectance * D * G * F / (4 * NdotV) + base.fr * (1.0 - F);
    result.ft = base.ft * (1.0 - F);
}
