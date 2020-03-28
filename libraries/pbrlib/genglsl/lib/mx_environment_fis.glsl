#include "pbrlib/genglsl/lib/mx_bsdfs.glsl"

// https://www.graphics.rwth-aachen.de/publication/2/jgt.pdf
float mx_golden_ratio_sequence(int i)
{
    return fract((float(i) + 1.0) * M_GOLDEN_RATIO);
}

// https://people.irisa.fr/Ricardo.Marques/articles/2013/SF_CGF.pdf
vec2 mx_spherical_fibonacci(int i, int numSamples)
{
    return vec2((float(i) + 0.5) / float(numSamples), mx_golden_ratio_sequence(i));
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
vec3 mx_environment_radiance(vec3 N, vec3 V, vec3 X, vec2 roughness, int distribution)
{
    vec3 Y = normalize(cross(N, X));
    X = cross(Y, N);

    // Compute shared dot products.
    float NdotV = clamp(dot(N, V), 1e-8, 1.0);
    
    // Integrate outgoing radiance using filtered importance sampling.
    // http://cgg.mff.cuni.cz/~jaroslav/papers/2008-egsr-fis/2008-egsr-fis-final-embedded.pdf
    vec3 radiance = vec3(0.0);
    for (int i = 0; i < $envRadianceSamples; i++)
    {
        vec2 Xi = mx_spherical_fibonacci(i, $envRadianceSamples);

        // Compute the half vector and incoming light direction.
        vec3 H = mx_microfacet_ggx_IS(Xi, X, Y, N, roughness.x, roughness.y);
        vec3 L = -reflect(V, H);
        
        // Compute dot products for this sample.
        float NdotH = clamp(dot(N, H), 1e-8, 1.0);
        float NdotL = clamp(dot(N, L), 1e-8, 1.0);
        float VdotH = clamp(dot(V, H), 1e-8, 1.0);
        float LdotH = VdotH;

        // Sample the environment light from the given direction.
        float pdf = mx_microfacet_ggx_PDF(X, Y, H, NdotH, LdotH, roughness.x, roughness.y);
        float lod = mx_latlong_compute_lod(L, pdf, $envRadianceMips - 1, $envRadianceSamples);
        vec3 sampleColor = mx_latlong_map_lookup(L, $envMatrix, lod, $envRadiance);

        // Compute the geometric term.
        float G = mx_microfacet_ggx_smith_G(NdotL, NdotV, max(roughness.x, roughness.y));
        
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
    radiance /= float($envRadianceSamples);
    return radiance;
}

vec3 mx_environment_irradiance(vec3 N)
{
    return mx_latlong_map_lookup(N, $envMatrix, 0.0, $envIrradiance);
}
