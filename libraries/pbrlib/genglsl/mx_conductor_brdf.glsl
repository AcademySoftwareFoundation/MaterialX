#include "pbrlib/genglsl/lib/mx_bsdfs.glsl"
#include "pbrlib/genglsl/lib/mx_refraction_index.glsl"

void mx_conductor_brdf_reflection(vec3 L, vec3 V, float weight, vec3 reflectivity, vec3 edgecolor, vec2 roughness, vec3 N, vec3 X, int distribution, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    float NdotL = dot(N,L);
    float NdotV = dot(N,V);
    if (NdotL <= 0.0 || NdotV <= 0.0)
    {
        result = BSDF(0.0);
        return;
    }

    vec3 Y = normalize(cross(N, X));

    vec3 H = normalize(L + V);
    float NdotH = dot(N, H);

    float D = mx_microfacet_ggx_NDF(X, Y, H, NdotH, roughness.x, roughness.y);
    float G = mx_microfacet_ggx_smith_G(NdotL, NdotV, max(roughness.x, roughness.y));

    vec3 ior_n, ior_k;
    mx_artistic_to_complex_ior(reflectivity, edgecolor, ior_n, ior_k);

    float VdotH = dot(V, H);
    vec3 F = mx_fresnel_conductor(VdotH, ior_n, ior_k);
    F *= weight;

    // Note: NdotL is cancelled out
    result = F * D * G / (4 * NdotV);
}

void mx_conductor_brdf_indirect(vec3 V, float weight, vec3 reflectivity, vec3 edgecolor, vec2 roughness, vec3 N, vec3 X, int distribution, out vec3 result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = vec3(0.0);
        return;
    }

    vec3 ior_n, ior_k;
    mx_artistic_to_complex_ior(reflectivity, edgecolor, ior_n, ior_k);

    vec3 Li = mx_environment_radiance(N, V, X, roughness, distribution);
    vec3 F = mx_fresnel_conductor(dot(N, V), ior_n, ior_k);
    F *= weight;
    result = Li * F;
}
