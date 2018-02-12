#include "sx/impl/shadergen/glsl/source/shadingmodels.glsl"

void sx_coatingbsdf(vec3 L, vec3 V, vec3 reflectance, float ior, float roughness, float anisotropy, vec3 normal, vec3 tangent, int distribution, BSDF base, out BSDF result)
{
    result = base;

    float NdotL = dot(normal,L);
    float NdotV = dot(normal,V);
    if (NdotL < 0 || NdotV < 0)
        return;

    vec3 H = normalize(L+V);
    float NdotH = dot(normal,H);

    float alpha = roughness * roughness;

    float D = sx_ggx_D(NdotH, alpha);
    float G = sx_smith_G(NdotL, NdotV, alpha);
    float F = sx_fresnel_schlick_roughness(NdotH, ior, alpha);

    result.fr = reflectance * D * G * F * NdotH / (4*NdotV*NdotL) + base.fr * (1.0 - F);
    result.ft = base.ft * (1.0 - F);
}
