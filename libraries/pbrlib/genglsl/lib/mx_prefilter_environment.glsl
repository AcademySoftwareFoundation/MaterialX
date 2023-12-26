#include "mx_microfacet_specular.glsl"

// This is the inverse calculation of `mx_latlong_compute_lod` in `mx_environment_prefilter.glsl`.
float mx_lod_to_alpha(float lod)
{
    float lodBias = lod / float($envRadianceMips);
    if (lodBias < 0.5)
    {
        return lodBias * lodBias;
    }
    else
    {
        return 2.0 * (lodBias - 0.375);
    }
}

vec3 mx_latlong_map_projection_inverse(vec2 uv)
{
    float latitude = (uv.y - 0.5) * M_PI;
    float longitude = (uv.x - 0.5) * M_PI * 2.0;

    float x = -cos(latitude) * sin(longitude);
    float y = -sin(latitude);
    float z = cos(latitude) * cos(longitude);

    return vec3(x, y, z);
}

// https://graphics.pixar.com/library/OrthonormalB/paper.pdf
mat3 mx_orthonormal_basis(vec3 N)
{
    float sign = (N.z < 0.0) ? -1.0 : 1.0;
    float a = -1.0 / (sign + N.z);
    float b = N.x * N.y * a;
    vec3 X = vec3(1.0 + sign * N.x * N.x * a, sign * b, -sign * N.x);
    vec3 Y = vec3(b, sign + N.y * N.y * a, -N.y);
    return mat3(X, Y, N);
}

vec3 mx_prefilter_environment()
{
    vec2 uv = gl_FragCoord.xy * pow(2.0, $envPrefilterMip) / vec2(2048.0, 1024.0);
    float alpha = mx_lod_to_alpha(float($envPrefilterMip));
    if ($envPrefilterMip == 0)
    {
        return textureLod($envRadiance, uv, 0).rgb;
    }

    // Compute world normal and transform.
    vec3 worldN = mx_latlong_map_projection_inverse(uv);
    mat3 localToWorld = mx_orthonormal_basis(worldN);

    // Local normal and view vectors are constant and aligned.
    vec3 V = vec3(0.0, 0.0, 1.0);
    float NdotV = 1.0;

    // If we use prefiltering, we can have a smaller sample count, since pre-filtering will reduce
    // the variance of the samples by choosing higher mip levels where necessary. We haven't
    // implemented prefiltering yet, so we use a high sample count.
    int envRadianceSamples = 1597; // Must be a Fibonacci number

    vec3 radiance = vec3(0.0, 0.0, 0.0);
    float weight = 0.0;
    for (int i = 0; i < envRadianceSamples; i++)
    {
        vec2 Xi = mx_spherical_fibonacci(i, envRadianceSamples);

        // Compute the half vector and incoming light direction.
        vec3 H = mx_ggx_importance_sample_VNDF(Xi, V, vec2(alpha, alpha));
        vec3 L = -V + 2.0 * H.z * H;

        // Compute dot products for this sample.
        float NdotL = clamp(L.z, M_FLOAT_EPS, 1.0);

        // Compute the geometric term.
        float G = mx_ggx_smith_G2(NdotL, NdotV, alpha);

        // Add the radiance contribution of this sample.
        vec3 sampleColor = mx_latlong_map_lookup(localToWorld * L, $envMatrix, 0, $envRadiance);
        radiance += G * sampleColor;
        weight += G;
    }

    return radiance / weight;
}
