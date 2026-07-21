#include "mx_microfacet_specular.wgsl"
// This shader cannot be pre-transpiled without generator specific binding information.

// Return the alpha associated with the given mip level in a prefiltered environment.
fn mx_latlong_lod_to_alpha(lod: f32) -> f32 
{
    let lodBias = lod / f32($envRadianceMips - 1);
    return select(2.0 * (lodBias - 0.375), mx_square_f32(lodBias), lodBias < 0.5);
}

// The inverse of mx_latlong_projection.
fn mx_latlong_map_projection_inverse(uv: vec2f) -> vec3f 
{
    let latitude = (uv.y - 0.5) * M_PI;
    let longitude = (uv.x - 0.5) * M_PI * 2.0;

    let x = -cos(latitude) * sin(longitude);
    let y = -sin(latitude);
    let z = cos(latitude) * cos(longitude);

    return vec3f(x, y, z);
}

// `fragCoord` is the pixel center (gl_FragCoord.xy equivalent); 
// `envRadianceSize` is the radiancemap's base-mip size in texels (textureDimensions of the radiance map).
fn mx_generate_prefilter_env(fragCoord: vec2f, envRadianceSize: vec2f) -> vec3f 
{
    // The tangent view vector is aligned with the normal.
    let V = vec3f(0.0, 0.0, 1.0);
    let NdotV = 1.0;

    // Compute derived properties.
    let uv = fragCoord * pow(2.0, f32($envPrefilterMip)) / envRadianceSize;
    let worldN = mx_latlong_map_projection_inverse(uv);
    let tangentToWorld = mx_orthonormal_basis(worldN);
    let alpha = mx_latlong_lod_to_alpha(f32($envPrefilterMip));
    let G1V = mx_ggx_smith_G1(NdotV, alpha);

    // Integrate the LD term for the given environment and alpha.
    var radiance = vec3f(0.0);
    var weight = 0.0;
    let envRadianceSamples: i32 = 1024;
    for (var i: i32 = 0; i < envRadianceSamples; i++) 
    {
        let Xi = mx_spherical_fibonacci(i, envRadianceSamples);

        // Compute the half vector and incoming light direction.
        let H = mx_ggx_importance_sample_VNDF(Xi, V, vec2f(alpha));
        let L = -V + 2.0 * H.z * H;

        // Compute the geometric term for this sample.
        let NdotL = clamp(L.z, M_FLOAT_EPS, 1.0);
        let G = mx_ggx_smith_G2(NdotL, NdotV, alpha);

        // Sample the environment light from the given direction.
        let Lw = tangentToWorld * L;
        let pdf = mx_ggx_VNDF_reflection_PDF(H, vec2f(alpha), G1V, NdotV);
        let lod = mx_latlong_compute_lod(Lw, pdf, f32($envRadianceMips - 1), envRadianceSamples);
        let sampleColor = mx_latlong_map_lookup(Lw, $envMatrix, lod, $envRadiance, $envRadianceSampler);

        // Add the radiance contribution of this sample.
        radiance += G * sampleColor;
        weight += G;
    }

    return radiance / weight;
}
