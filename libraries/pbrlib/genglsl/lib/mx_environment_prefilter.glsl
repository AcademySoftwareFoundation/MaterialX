#include "pbrlib/genglsl/lib/mx_microfacet_specular.glsl"

float mx_latlong_compute_lod(float roughness)
{
    // Select a mip level based on input roughness.
    float lodBias = roughness < 0.25 ? sqrt(roughness) : 0.5*roughness + 0.375;
    return lodBias * $envRadianceMips;
}

vec3 mx_environment_radiance(vec3 N, vec3 V, vec3 X, vec2 roughness, int distribution, FresnelData fd)
{
    N = mx_forward_facing_normal(N, V);
    vec3 L = reflect(-V, N);

    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    float avgRoughness = mx_average_roughness(roughness);
    vec3 F = mx_compute_fresnel(NdotV, fd);
    float G = mx_ggx_smith_G(NdotV, NdotV, avgRoughness);
    vec3 comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);
    vec3 Li = mx_latlong_map_lookup(L, $envMatrix, mx_latlong_compute_lod(avgRoughness), $envRadiance);

    return Li * F * G * comp;
}

vec3 mx_environment_irradiance(vec3 N)
{
    return mx_latlong_map_lookup(N, $envMatrix, 0.0, $envIrradiance);
}
