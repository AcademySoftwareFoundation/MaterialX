#include "mx_microfacet_specular.wgsl"
// This shader cannot be pre-transpiled without generator specific binding information.

fn mx_environment_radiance(N: vec3f, V_in: vec3f, X_in: vec3f, alpha: vec2f, distribution: i32, fd: FresnelData) -> vec3f 
{
    // Generate tangent frame.
    let X = normalize(X_in - dot(X_in, N) * N);
    let Y = cross(N, X);
    let tangentToWorld = mat3x3f(X, Y, N);

    // Transform the view vector to tangent space.
    let V = vec3f(dot(V_in, X), dot(V_in, Y), dot(V_in, N));

    // Compute derived properties.
    let NdotV = clamp(V.z, M_FLOAT_EPS, 1.0);
    let avgAlpha = mx_average_alpha(alpha);
    let G1V = mx_ggx_smith_G1(NdotV, avgAlpha);

    // Integrate outgoing radiance using filtered importance sampling.
    // http://cgg.mff.cuni.cz/~jaroslav/papers/2008-egsr-fis/2008-egsr-fis-final-embedded.pdf
    var radiance = vec3f(0.0);
    let envRadianceSamples: i32 = $envRadianceSamples;
    for (var i: i32 = 0; i < envRadianceSamples; i++) 
    {
        let Xi = mx_spherical_fibonacci(i, envRadianceSamples);

        // Compute the half vector and incoming light direction.
        let H = mx_ggx_importance_sample_VNDF(Xi, V, alpha);
        // For surface transmission, follow the refracted ray through a solid sphere; otherwise reflect.
        let L = select(-reflect(V, H), mx_refraction_solid_sphere(-V, H, fd.ior.x), fd.refraction);
        // Compute dot products for this sample.
        let NdotL = clamp(L.z, M_FLOAT_EPS, 1.0);
        let VdotH = clamp(dot(V, H), M_FLOAT_EPS, 1.0);

        // Sample the environment light from the given direction.
        let Lw = tangentToWorld * L;
        let pdf = mx_ggx_NDF(H, alpha) * G1V / (4.0 * NdotV);
        let lod = mx_latlong_compute_lod(Lw, pdf, f32($envRadianceMips - 1), envRadianceSamples);
        let sampleColor = mx_latlong_map_lookup(Lw, $envMatrix, lod, $envRadiance, $envRadianceSampler);

        // Compute the Fresnel term.
        let F = mx_compute_fresnel(VdotH, fd);

        // Compute the geometric term.
        let G = mx_ggx_smith_G2(NdotL, NdotV, avgAlpha);

        // The combined FG term simplifies to inverted Fresnel for refraction.
        let FG = select(F * G, vec3f(1.0) - F, fd.refraction);

        // Add the radiance contribution of this sample.
        // From https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
        //   incidentLight = sampleColor * NdotL
        //   microfacetSpecular = D * F * G / (4 * NdotL * NdotV)
        //   pdf = D * G1V / (4 * NdotV);
        //   radiance = incidentLight * microfacetSpecular / pdf
        radiance += sampleColor * FG;
    }

    // Apply the global component of the geometric term and normalize.
    radiance /= G1V * f32(envRadianceSamples);

    // Return the final radiance.
    if ($envRadianceSamples == 0) {
        return vec3f(0.0);
    }
    return radiance * $envLightIntensity;
}

fn mx_environment_irradiance(N: vec3f) -> vec3f {
    let Li = mx_latlong_map_lookup(N, $envMatrix, 0.0, $envIrradiance, $envIrradianceSampler);
    return Li * $envLightIntensity;
}
