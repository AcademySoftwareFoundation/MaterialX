#include "pbrlib/genglsl/lib/mx_microfacet_specular.glsl"

void mx_generalized_schlick_brdf_reflection(vec3 L, vec3 V, float weight, vec3 color0, vec3 color90, float exponent, vec2 roughness, vec3 N, vec3 X, int distribution, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    vec3 Y = normalize(cross(N, X));
    vec3 H = normalize(L + V);

    float NdotL = clamp(dot(N, L), M_FLOAT_EPS, 1.0);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);
    float NdotH = clamp(dot(N, H), M_FLOAT_EPS, 1.0);
    float VdotH = clamp(dot(V, H), M_FLOAT_EPS, 1.0);

    float avgRoughness = mx_average_roughness(roughness);

    float D = mx_ggx_NDF(X, Y, H, NdotH, roughness.x, roughness.y);
    vec3 F = mx_fresnel_schlick(VdotH, color0, color90, exponent);
    float G = mx_ggx_smith_G(NdotL, NdotV, avgRoughness);

    vec3 comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);
    vec3 dirAlbedo = mx_ggx_directional_albedo(NdotV, avgRoughness, color0, color90) * comp;
    float avgDirAlbedo = dot(dirAlbedo, vec3(1.0 / 3.0));

    // Note: NdotL is cancelled out
    result = D * F * G * comp * weight / (4 * NdotV)    // Top layer reflection
           + base * (1.0 - avgDirAlbedo * weight);      // Base layer reflection attenuated by top layer
}

void mx_generalized_schlick_brdf_transmission(vec3 V, float weight, vec3 color0, vec3 color90, float exponent, vec2 roughness, vec3 N, vec3 X, int distribution, BSDF base, out BSDF result)
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
    float NdotV = abs(dot(N, V));

    float avgRoughness = mx_average_roughness(roughness);
    vec3 F = mx_fresnel_schlick(NdotV, color0, color90, exponent);

    vec3 comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);
    vec3 dirAlbedo = mx_ggx_directional_albedo(NdotV, avgRoughness, color0, color90) * comp;
    float avgDirAlbedo = dot(dirAlbedo, vec3(1.0 / 3.0));

    result = base * (1.0 - avgDirAlbedo * weight); // Base layer transmission attenuated by top layer
}

void mx_generalized_schlick_brdf_indirect(vec3 V, float weight, vec3 color0, vec3 color90, float exponent, vec2 roughness, vec3 N, vec3 X, int distribution, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    float avgRoughness = mx_average_roughness(roughness);
    vec3 F = mx_fresnel_schlick(NdotV, color0, color90, exponent);

    vec3 comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);
    vec3 dirAlbedo = mx_ggx_directional_albedo(NdotV, avgRoughness, color0, color90) * comp;
    float avgDirAlbedo = dot(dirAlbedo, vec3(1.0 / 3.0));

    vec3 Li = mx_environment_radiance(N, V, X, roughness, color0, color90, vec3(1.0), vec3(1.0), distribution, 0);

    result = Li * comp * weight                     // Top layer reflection
           + base * (1.0 - avgDirAlbedo * weight);  // Base layer reflection attenuated by top layer
}
