#include "mx_microfacet.glsl"

const float FUJII_CONSTANT_1 = 0.5 - 2.0 / (3.0 * M_PI);
const float FUJII_CONSTANT_2 = 2.0 / 3.0 - 28.0 / (15.0 * M_PI);

// Qualitative Oren-Nayar diffuse with simplified math:
// https://www1.cs.columbia.edu/CAVE/publications/pdfs/Oren_SIGGRAPH94.pdf
float mx_oren_nayar_diffuse(float NdotV, float NdotL, float LdotV, float roughness)
{
    float s = LdotV - NdotL * NdotV;
    float stinv = (s > 0.0) ? s / max(NdotL, NdotV) : 0.0;

    float sigma2 = mx_square(roughness);
    float A = 1.0 - 0.5 * (sigma2 / (sigma2 + 0.33));
    float B = 0.45 * sigma2 / (sigma2 + 0.09);

    return A + B * stinv;
}

// Rational quadratic fit to Monte Carlo data for Oren-Nayar directional albedo.
float mx_oren_nayar_diffuse_dir_albedo_analytic(float NdotV, float roughness)
{
    vec2 r = vec2(1.0, 1.0) +
             vec2(-0.4297, -0.6076) * roughness +
             vec2(-0.7632, -0.4993) * NdotV * roughness +
             vec2(1.4385, 2.0315) * mx_square(roughness);
    return r.x / r.y;
}

float mx_oren_nayar_diffuse_dir_albedo_table_lookup(float NdotV, float roughness)
{
#if DIRECTIONAL_ALBEDO_METHOD == 1
    if (textureSize($albedoTable, 0).x > 1)
    {
        return texture($albedoTable, vec2(NdotV, roughness)).b;
    }
#endif
    return 0.0;
}

float mx_oren_nayar_diffuse_dir_albedo_monte_carlo(float NdotV, float roughness)
{
    NdotV = clamp(NdotV, M_FLOAT_EPS, 1.0);
    vec3 V = vec3(sqrt(1.0 - mx_square(NdotV)), 0, NdotV);

    float radiance = 0.0;
    const int SAMPLE_COUNT = 64;
    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
        vec2 Xi = mx_spherical_fibonacci(i, SAMPLE_COUNT);

        // Compute the incoming light direction.
        vec3 L = mx_uniform_sample_hemisphere(Xi);
        
        // Compute dot products for this sample.
        float NdotL = clamp(L.z, M_FLOAT_EPS, 1.0);
        float LdotV = clamp(dot(L, V), M_FLOAT_EPS, 1.0);

        // Compute diffuse reflectance.
        float reflectance = mx_oren_nayar_diffuse(NdotV, NdotL, LdotV, roughness);

        // Add the radiance contribution of this sample.
        //   uniform_pdf = 1 / (2 * PI)
        //   radiance = (reflectance * NdotL) / (uniform_pdf * PI);
        radiance += reflectance * NdotL;
    }

    // Apply global components and normalize.
    radiance *= 2.0 / float(SAMPLE_COUNT);

    // Return the final directional albedo.
    return radiance;
}

float mx_oren_nayar_diffuse_dir_albedo(float NdotV, float roughness)
{
#if DIRECTIONAL_ALBEDO_METHOD == 2
    float dirAlbedo = mx_oren_nayar_diffuse_dir_albedo_monte_carlo(NdotV, roughness);
#else
    float dirAlbedo = mx_oren_nayar_diffuse_dir_albedo_analytic(NdotV, roughness);
#endif
    return clamp(dirAlbedo, 0.0, 1.0);
}

// Improved Oren-Nayar diffuse from Fujii:
// https://mimosa-pudica.net/improved-oren-nayar.html
float mx_oren_nayar_fujii_diffuse_dir_albedo(float cosTheta, float roughness)
{
    float A = 1.0 / (1.0 + FUJII_CONSTANT_1 * roughness);
    float B = roughness * A;
    float Si = sqrt(max(0.0, 1.0 - mx_square(cosTheta)));
    float G = Si * (mx_acos(clamp(cosTheta, -1.0, 1.0)) - Si * cosTheta) +
              2.0 * ((Si / cosTheta) * (1.0 - Si * Si * Si) - Si) / 3.0;
    return A + (B * G * M_PI_INV);
}

float mx_oren_nayar_fujii_diffuse_avg_albedo(float roughness)
{
    float A = 1.0 / (1.0 + FUJII_CONSTANT_1 * roughness);
    return A * (1.0 + FUJII_CONSTANT_2 * roughness);
}   

// Energy-compensated Oren-Nayar diffuse from OpenPBR Surface:
// https://academysoftwarefoundation.github.io/OpenPBR/
vec3 mx_oren_nayar_compensated_diffuse(float NdotV, float NdotL, float LdotV, float roughness, vec3 color)
{
    float s = LdotV - NdotL * NdotV;
    float stinv = (s > 0.0) ? s / max(NdotL, NdotV) : s;

    // Compute the single-scatter lobe.
    float A = 1.0 / (1.0 + FUJII_CONSTANT_1 * roughness);
    vec3 lobeSingleScatter = color * A * (1.0 + roughness * stinv);

    // Compute the multi-scatter lobe.
    float dirAlbedoV = mx_oren_nayar_fujii_diffuse_dir_albedo(NdotV, roughness);
    float dirAlbedoL = mx_oren_nayar_fujii_diffuse_dir_albedo(NdotL, roughness);
    float avgAlbedo = mx_oren_nayar_fujii_diffuse_avg_albedo(roughness);
    vec3 colorMultiScatter = mx_square(color) * avgAlbedo /
                             (vec3(1.0) - color * max(0.0, 1.0 - avgAlbedo));
    vec3 lobeMultiScatter = colorMultiScatter *
                            max(M_FLOAT_EPS, 1.0 - dirAlbedoV) *
                            max(M_FLOAT_EPS, 1.0 - dirAlbedoL) /
                            max(M_FLOAT_EPS, 1.0 - avgAlbedo);

    // Return the sum.
    return lobeSingleScatter + lobeMultiScatter;
}

vec3 mx_oren_nayar_compensated_diffuse_dir_albedo(float cosTheta, float roughness, vec3 color)
{
    float dirAlbedo = mx_oren_nayar_fujii_diffuse_dir_albedo(cosTheta, roughness);
    float avgAlbedo = mx_oren_nayar_fujii_diffuse_avg_albedo(roughness);
    vec3 colorMultiScatter = mx_square(color) * avgAlbedo /
                             (vec3(1.0) - color * max(0.0, 1.0 - avgAlbedo));
    return mix(colorMultiScatter, color, dirAlbedo);
}
  
// https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf
// Section 5.3
float mx_burley_diffuse(float NdotV, float NdotL, float LdotH, float roughness)
{
    float F90 = 0.5 + (2.0 * roughness * mx_square(LdotH));
    float refL = mx_fresnel_schlick(NdotL, 1.0, F90);
    float refV = mx_fresnel_schlick(NdotV, 1.0, F90);
    return refL * refV;
}

// Compute the directional albedo component of Burley diffuse for the given
// view angle and roughness.  Curve fit provided by Stephen Hill.
float mx_burley_diffuse_dir_albedo(float NdotV, float roughness)
{
    float x = NdotV;
    float fit0 = 0.97619 - 0.488095 * mx_pow5(1.0 - x);
    float fit1 = 1.55754 + (-2.02221 + (2.56283 - 1.06244 * x) * x) * x;
    return mix(fit0, fit1, roughness);
}

// Evaluate the Burley diffusion profile for the given distance and diffusion shape.
// Based on https://graphics.pixar.com/library/ApproxBSSRDF/
vec3 mx_burley_diffusion_profile(float dist, vec3 shape)
{
    vec3 num1 = exp(-shape * dist);
    vec3 num2 = exp(-shape * dist / 3.0);
    float denom = max(dist, M_FLOAT_EPS);
    return (num1 + num2) / denom;
}

// Integrate the Burley diffusion profile over a sphere of the given radius.
// Inspired by Eric Penner's presentation in http://advances.realtimerendering.com/s2011/
vec3 mx_integrate_burley_diffusion(vec3 N, vec3 L, float radius, vec3 mfp)
{
    float theta = mx_acos(dot(N, L));

    // Estimate the Burley diffusion shape from mean free path.
    vec3 shape = vec3(1.0) / max(mfp, 0.1);

    // Integrate the profile over the sphere.
    vec3 sumD = vec3(0.0);
    vec3 sumR = vec3(0.0);
    const int SAMPLE_COUNT = 32;
    const float SAMPLE_WIDTH = (2.0 * M_PI) / float(SAMPLE_COUNT);
    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
        float x = -M_PI + (float(i) + 0.5) * SAMPLE_WIDTH;
        float dist = radius * abs(2.0 * mx_sin(x * 0.5));
        vec3 R = mx_burley_diffusion_profile(dist, shape);
        sumD += R * max(mx_cos(theta + x), 0.0);
        sumR += R;
    }

    return sumD / sumR;
}

vec3 mx_subsurface_scattering_approx(vec3 N, vec3 L, vec3 P, vec3 albedo, vec3 mfp)
{
    float curvature = length(fwidth(N)) / length(fwidth(P));
    float radius = 1.0 / max(curvature, 0.01);
    return albedo * mx_integrate_burley_diffusion(N, L, radius, mfp) / vec3(M_PI);
}
