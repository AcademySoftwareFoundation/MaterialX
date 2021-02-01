#include "pbrlib/genglsl/lib/mx_microfacet_specular.glsl"

void mx_generalized_schlick_bsdf_reflection(vec3 L, vec3 V, vec3 P, float occlusion, float weight, vec3 color0, vec3 color90, float exponent, vec2 roughness, vec3 N, vec3 X, int distribution, int scatter_mode, BSDF base, thinfilm tf, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    N = mx_forward_facing_normal(N, V);

    vec3 Y = normalize(cross(N, X));
    vec3 H = normalize(L + V);

    float NdotL = clamp(dot(N, L), M_FLOAT_EPS, 1.0);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);
    float NdotH = clamp(dot(N, H), M_FLOAT_EPS, 1.0);
    float VdotH = clamp(dot(V, H), M_FLOAT_EPS, 1.0);

    float avgRoughness = mx_average_roughness(roughness);

    FresnelData fd = mx_init_fresnel_schlick(color0, color90, exponent);
    vec3  F = mx_compute_fresnel(VdotH, fd);
    float D = mx_ggx_NDF(X, Y, H, NdotH, roughness.x, roughness.y);
    float G = mx_ggx_smith_G(NdotL, NdotV, avgRoughness);

    vec3 comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);
    vec3 dirAlbedo = mx_ggx_directional_albedo(NdotV, avgRoughness, color0, color90) * comp;
    float avgDirAlbedo = dot(dirAlbedo, vec3(1.0 / 3.0));

    // Note: NdotL is cancelled out
    result = D * F * G * comp * occlusion * weight / (4 * NdotV)    // Top layer reflection
           + base * (1.0 - avgDirAlbedo * weight);                  // Base layer reflection attenuated by top layer
}

void mx_generalized_schlick_bsdf_transmission(vec3 V, float weight, vec3 color0, vec3 color90, float exponent, vec2 roughness, vec3 N, vec3 X, int distribution, int scatter_mode, BSDF base, thinfilm tf, out BSDF result)
{
    //
    // TODO: We need handling of transmission for generalized schlick
    //

    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    N = mx_forward_facing_normal(N, V);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    FresnelData fd = mx_init_fresnel_schlick(color0, color90, exponent);
    vec3 F = mx_compute_fresnel(NdotV, fd);

    float avgRoughness = mx_average_roughness(roughness);
    vec3 comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);
    vec3 dirAlbedo = mx_ggx_directional_albedo(NdotV, avgRoughness, color0, color90) * comp;
    float avgDirAlbedo = dot(dirAlbedo, vec3(1.0 / 3.0));

    result = base * (1.0 - avgDirAlbedo * weight); // Transmission attenuated by top layer
}

void mx_generalized_schlick_bsdf_indirect(vec3 V, float weight, vec3 color0, vec3 color90, float exponent, vec2 roughness, vec3 N, vec3 X, int distribution, int scatter_mode, BSDF base, thinfilm tf, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    N = mx_forward_facing_normal(N, V);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    FresnelData fd = mx_init_fresnel_schlick(color0, color90, exponent);
    vec3 F = mx_compute_fresnel(NdotV, fd);

    float avgRoughness = mx_average_roughness(roughness);
    vec3 comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);
    vec3 dirAlbedo = mx_ggx_directional_albedo(NdotV, avgRoughness, color0, color90) * comp;
    float avgDirAlbedo = dot(dirAlbedo, vec3(1.0 / 3.0));

    vec3 Li = mx_environment_radiance(N, V, X, roughness, distribution, fd);

    result = Li * comp * weight                     // Top layer reflection
           + base * (1.0 - avgDirAlbedo * weight);  // Base layer reflection attenuated by top layer
}
