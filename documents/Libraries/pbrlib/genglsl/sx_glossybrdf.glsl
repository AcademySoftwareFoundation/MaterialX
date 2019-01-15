#include "sxpbrlib/sx-glsl/lib/sx_bsdfs.glsl"

void sx_glossybrdf_reflection(vec3 L, vec3 V, float weight, vec3 color0, vec3 color90, float exponent, roughnessinfo roughness, vec3 normal, vec3 tangent, int distribution, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    float NdotL = dot(normal,L);
    float NdotV = dot(normal,V);
    if (NdotL <= 0.0 || NdotV <= 0.0)
    {
        result = base;
        return;
    }

    vec3 bitangent = normalize(cross(normal, tangent));

    vec3 H = normalize(L + V);
    float NdotH = dot(normal, H);

    float D = sx_microfacet_ggx_NDF(tangent, bitangent, H, NdotH, roughness.alphaX, roughness.alphaY);
    float G = sx_microfacet_ggx_smith_G(NdotL, NdotV, roughness.alpha);

    float VdotH = dot(V, H);
    vec3 F = sx_fresnel_schlick(VdotH, color0, color90, exponent);
    F *= weight;
    float avgF = dot(F, vec3(1.0 / 3.0));

    // Note: NdotL is cancelled out
    result = D * G * F / (4 * NdotV)    // Top layer reflection
           + base * (1.0 - avgF);       // Base layer reflection attenuated by top fresnel
}

void sx_glossybrdf_transmission(vec3 V, float weight, vec3 color0, vec3 color90, float exponent, roughnessinfo roughness, vec3 normal, vec3 tangent, int distribution, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    // Glossy BRDF has no transmission but we must 
    // attenuate the base layer transmission by the 
    // inverse of top layer reflectance.

    // Abs here to allow transparency through backfaces
    float NdotV = abs(dot(normal,V)); 
    vec3 F = sx_fresnel_schlick(NdotV, color0, color90, exponent);
    F *= weight;
    float avgF = dot(F, vec3(1.0 / 3.0));

    result = base * (1.0 - avgF); // Base layer transmission attenuated by top fresnel
}

void sx_glossybrdf_indirect(vec3 V, float weight, vec3 color0, vec3 color90, float exponent, roughnessinfo roughness, vec3 normal, vec3 tangent, int distribution, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    vec3 Li = sx_environment_specular(normal, V, roughness.alpha);

    float NdotV = dot(normal,V);
    vec3 F = sx_fresnel_schlick(NdotV, color0, color90, exponent);
    F *= weight;
    float avgF = dot(F, vec3(1.0 / 3.0));

    result = Li * F                 // Top layer reflection
           + base * (1.0 - avgF);   // Base layer reflection attenuated by top fresnel
}
