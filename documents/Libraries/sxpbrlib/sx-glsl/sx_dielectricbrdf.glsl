#include "sxpbrlib/sx-glsl/lib/sx_bsdfs.glsl"

void sx_dielectricbrdf(vec3 L, vec3 V, float weight, vec3 color, float ior, vec2 roughness, vec3 normal, vec3 tangent, int distribution, BSDF base, out BSDF result)
{
    result = base;
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

    float VdotH = dot(V, H);
    float F = sx_fresnel_schlick(VdotH, ior);
    F *= weight;

    // Note: NdotL is cancelled out
    result = color * D * G * F / (4 * NdotV) // Specular coating component
           + base * (1.0 - F);               // Base component attenuated by coating fresnel
}

void sx_dielectricbrdf_ibl(vec3 V, float weight, vec3 color, float ior, vec2 roughness, vec3 normal, vec3 tangent, int distribution, vec3 base, out vec3 result)
{
    result = base;
    if (weight < M_FLOAT_EPS)
        return;

    vec3 Li = sx_environment_specular(normal, V, roughness.x);
    float F = sx_fresnel_schlick_roughness(dot(normal, V), ior, roughness.x);
    F *= weight;

    result = Li * color * F     // Specular coating component
           + base * (1.0 - F);  // Base component attenuated by coating fresnel
}
