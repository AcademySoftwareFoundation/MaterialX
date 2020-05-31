#include "pbrlib/genglsl/lib/mx_microfacet_specular.glsl"
#include "pbrlib/genglsl/lib/mx_refraction_index.glsl"

void mx_conductor_brdf_reflection(vec3 L, vec3 V, float weight, vec3 reflectivity, vec3 edge_color, vec2 roughness, vec3 N, vec3 X, int distribution, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    vec3 Y = normalize(cross(N, X));
    vec3 H = normalize(L + V);

    float NdotL = clamp(dot(N, L), M_FLOAT_EPS, 1.0);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);
    float NdotH = clamp(dot(N, H), M_FLOAT_EPS, 1.0);
    float VdotH = clamp(dot(V, H), M_FLOAT_EPS, 1.0);

    vec3 ior_n, ior_k;
    mx_artistic_to_complex_ior(reflectivity, edge_color, ior_n, ior_k);

    float avgRoughness = mx_average_roughness(roughness);

    float D = mx_ggx_NDF(X, Y, H, NdotH, roughness.x, roughness.y);
    vec3 F = mx_fresnel_conductor(VdotH, ior_n, ior_k);
    float G = mx_ggx_smith_G(NdotL, NdotV, avgRoughness);

    vec3 comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);

    // Note: NdotL is cancelled out
    result = D * F * G * comp * weight / (4 * NdotV);
}

void mx_conductor_brdf_indirect(vec3 V, float weight, vec3 reflectivity, vec3 edge_color, vec2 roughness, vec3 N, vec3 X, int distribution, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    vec3 ior_n, ior_k;
    mx_artistic_to_complex_ior(reflectivity, edge_color, ior_n, ior_k);

    float avgRoughness = mx_average_roughness(roughness);
    vec3 F = mx_fresnel_conductor(NdotV, ior_n, ior_k);
    vec3 comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);

    vec3 Li = mx_environment_radiance(N, V, X, roughness, vec3(1.0), vec3(1.0), ior_n, ior_k, distribution, 1);

    result = Li * comp * weight;
}
