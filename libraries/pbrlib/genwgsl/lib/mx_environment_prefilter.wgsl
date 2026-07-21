#include "mx_microfacet_specular.wgsl"
// This shader cannot be pre-transpiled without generator specific binding information.

// Return the mip level associated with the given alpha in a prefiltered environment.
fn mx_latlong_alpha_to_lod(alpha: f32) -> f32
{
    let lodBias = select(0.5 * alpha + 0.375, sqrt(alpha), alpha < 0.25);
    return lodBias * f32($envRadianceMips - 1);
}

fn mx_environment_radiance(N_in: vec3f, V: vec3f, X: vec3f, alpha: vec2f, distribution: i32, fd: FresnelData) -> vec3f
{
    let N = mx_forward_facing_normal(N_in, V);
    let L = select(-reflect(V, N), mx_refraction_solid_sphere(-V, N, fd.ior.x), fd.refraction);

    let NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    let avgAlpha = mx_average_alpha(alpha);
    var FG = mx_ggx_dir_albedo_fresnel(NdotV, avgAlpha, fd);
    FG = select(FG, vec3f(1.0) - FG, fd.refraction);

    let Li = mx_latlong_map_lookup(L, $envMatrix, mx_latlong_alpha_to_lod(avgAlpha), $envRadiance, $envRadianceSampler);
    return Li * FG * $envLightIntensity;
}

fn mx_environment_irradiance(N: vec3f) -> vec3f
{
    let Li = mx_latlong_map_lookup(N, $envMatrix, 0.0, $envIrradiance, $envIrradianceSampler);
    return Li * $envLightIntensity;
}
