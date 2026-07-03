#include "mx_microfacet_specular.wgsl"

// Filtered Importance Sampling environment lighting.
// http://cgg.mff.cuni.cz/~jaroslav/papers/2008-egsr-fis/2008-egsr-fis-final-embedded.pdf
//
// Iterates envRadianceSamples per pixel with GGX VNDF importance sampling.
//
// Requires (provided by the generator's shader template):
//   envRadiance      : texture_2d<f32>   — lat-long radiance environment map
//   envRadianceSampler : sampler
//   envIrradiance    : texture_2d<f32>   — lat-long irradiance environment map
//   envIrradianceSampler : sampler
//   envRadianceMips  : i32               — number of mip levels in envRadiance
//   envRadianceSamples : i32             — number of importance samples
//   envMatrix        : mat4x4f       — environment rotation matrix
//   envLightIntensity : f32              — environment light intensity multiplier
//
// Dependencies from mx_microfacet_specular.wgsl:
//   mx_average_alpha, mx_ggx_smith_G1, mx_ggx_smith_G2, mx_ggx_NDF,
//   mx_ggx_importance_sample_VNDF, mx_compute_fresnel,
//   mx_refraction_solid_sphere, mx_latlong_map_lookup, mx_latlong_compute_lod
// Dependencies from mx_microfacet.wgsl:
//   mx_spherical_fibonacci

fn mx_environment_radiance(N: vec3f, V_in: vec3f, X_in: vec3f, alpha: vec2f, distribution: i32, fd: FresnelData) -> vec3f {
    let X = normalize(X_in - dot(X_in, N) * N);
    let Y = cross(N, X);
    let tangentToWorld = mat3x3f(X, Y, N);

    let V = vec3f(dot(V_in, X), dot(V_in, Y), dot(V_in, N));

    let NdotV = clamp(V.z, M_FLOAT_EPS, 1.0);
    let avgAlpha = mx_average_alpha(alpha);
    let G1V = mx_ggx_smith_G1(NdotV, avgAlpha);

    var radiance = vec3f(0.0);
    let envRadianceSamples: i32 = $envRadianceSamples;
    for (var i: i32 = 0; i < envRadianceSamples; i++) {
        let Xi = mx_spherical_fibonacci(i, envRadianceSamples);

        let H = mx_ggx_importance_sample_VNDF(Xi, V, alpha);
        // For surface transmission, follow the refracted ray through a solid sphere; otherwise reflect.
        let L = select(-reflect(V, H), mx_refraction_solid_sphere(-V, H, fd.ior.x), fd.refraction);

        let NdotL = clamp(L.z, M_FLOAT_EPS, 1.0);
        let VdotH = clamp(dot(V, H), M_FLOAT_EPS, 1.0);

        let Lw = tangentToWorld * L;
        let pdf = mx_ggx_NDF(H, alpha) * G1V / (4.0 * NdotV);
        let lod = mx_latlong_compute_lod(Lw, pdf, f32($envRadianceMips - 1), envRadianceSamples);
        let sampleColor = mx_latlong_map_lookup(Lw, $envMatrix, lod, $envRadiance, $envRadianceSampler);

        let F = mx_compute_fresnel(VdotH, fd);
        let G = mx_ggx_smith_G2(NdotL, NdotV, avgAlpha);
        // The combined FG term simplifies to inverted Fresnel for refraction.
        let FG = select(F * G, vec3f(1.0) - F, fd.refraction);

        radiance += sampleColor * FG;
    }

    radiance /= G1V * f32(envRadianceSamples);

    if ($envRadianceSamples == 0) {
        return vec3f(0.0);
    }
    return radiance * $envLightIntensity;
}

fn mx_environment_irradiance(N: vec3f) -> vec3f {
    let Li = mx_latlong_map_lookup(N, $envMatrix, 0.0, $envIrradiance, $envIrradianceSampler);
    return Li * $envLightIntensity;
}
