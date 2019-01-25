#include "pbrlib/genglsl/lib/mx_bsdfs.glsl"

vec2 mx_latlong_projection(vec3 dir)
{
    float latitude = -asin(dir.y) * M_PI_INV + 0.5;
    latitude = clamp(latitude, 0.01, 0.99);
    float longitude = atan(dir.x, -dir.z) * M_PI_INV * 0.5 + 0.5;
    return vec2(longitude, latitude);
}

// https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html
// Section 20.4 Equation 13
float mx_latlong_compute_lod(vec3 dir, float pdf, float maxMipLevel, int envSamples)
{
    const float MIP_LEVEL_OFFSET = 1.5;
    float effectiveMaxMipLevel = maxMipLevel - MIP_LEVEL_OFFSET;
    float distortion = sqrt(1.0 - mx_square(dir.y));
    return max(effectiveMaxMipLevel - 0.5 * log2(envSamples * pdf * distortion), 0.0);
}

vec3 mx_latlong_map_lookup(vec3 dir, mat4 transform, float lod, sampler2D sampler)
{
    vec3 envDir = normalize((transform * vec4(dir,0.0)).xyz);
    vec2 uv = mx_latlong_projection(envDir);
    return textureLod(sampler, uv, lod).rgb;
}

// Only GGX is supported for now and the distribution argument is ignored
vec3 mx_environment_radiance(vec3 N, vec3 V, vec3 X, roughnessinfo roughness, int distribution)
{
    vec3 Y = normalize(cross(N, X));
    X = cross(Y, N);

    // Compute shared dot products.
    float NdotV = clamp(dot(N, V), 1e-8, 1.0);
    
    // Integrate outgoing radiance using filtered importance sampling.
    // http://cgg.mff.cuni.cz/~jaroslav/papers/2008-egsr-fis/2008-egsr-fis-final-embedded.pdf
    vec3 radiance = vec3(0.0);
    for (int i = 0; i < u_envSamples; i++)
    {
        vec2 Xi = mx_spherical_fibonacci(i, u_envSamples);

        // Compute the half vector and incoming light direction.
        vec3 H = mx_microfacet_ggx_IS(Xi, X, Y, N, roughness.alphaX, roughness.alphaY);
        vec3 L = -reflect(V, H);
        
        // Compute dot products for this sample.
        float NdotH = clamp(dot(N, H), 1e-8, 1.0);
        float NdotL = clamp(dot(N, L), 1e-8, 1.0);
        float VdotH = clamp(dot(V, H), 1e-8, 1.0);
        float LdotH = VdotH;

        // Sample the environment light from the given direction.
        float pdf = mx_microfacet_ggx_PDF(X, Y, H, NdotH, LdotH, roughness.alphaX, roughness.alphaY);
        float lod = mx_latlong_compute_lod(L, pdf, u_envRadianceMips - 1, u_envSamples);
        vec3 sampleColor = mx_latlong_map_lookup(L, u_envMatrix, lod, u_envRadiance);

        // Compute the geometric term.
        float G = mx_microfacet_ggx_smith_G(NdotL, NdotV, roughness.alpha);
        
        // Fresnel is applied outside the lighting integral for now.
        // TODO: Move Fresnel term into the lighting integral.
        float F = 1.0;

        // Add the radiance contribution of this sample.
        // From https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
        //   incidentLight = sampleColor * NdotL
        //   microfacetSpecular = D * G * F / (4 * NdotL * NdotV)
        //   pdf = D * NdotH / (4 * VdotH)
        //   radiance = incidentLight * microfacetSpecular / pdf
        radiance += sampleColor * G * F * VdotH / (NdotV * NdotH);
    }

    // Normalize and return the final radiance.
    radiance /= float(u_envSamples);
    return radiance;
}

vec3 mx_environment_irradiance(vec3 N)
{
    return mx_latlong_map_lookup(N, u_envMatrix, 0.0, u_envIrradiance);
}
