#include "pbrlib/genglsl/lib/mx_microfacet.glsl"

// Based on the OSL implementation of Oren-Nayar diffuse, which is in turn
// based on https://mimosa-pudica.net/improved-oren-nayar.html.
float mx_oren_nayar_diffuse(vec3 L, vec3 V, vec3 N, float NdotL, float roughness)
{
    float LdotV = clamp(dot(L, V), M_FLOAT_EPS, 1.0);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);
    float s = LdotV - NdotL * NdotV;
    float stinv = (s > 0.0f) ? s / max(NdotL, NdotV) : 0.0;

    float sigma2 = mx_square(roughness * M_PI);
    float A = 1.0 - 0.5 * (sigma2 / (sigma2 + 0.33));
    float B = 0.45 * sigma2 / (sigma2 + 0.09);

    return A + B * stinv;
}

// https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
// Section 5.3
float mx_burley_diffuse(vec3 L, vec3 V, vec3 N, float NdotL, float roughness)
{
    vec3 H = normalize(L + V);
    float LdotH = clamp(dot(L, H), M_FLOAT_EPS, 1.0);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    float F90 = 0.5 + (2.0 * roughness * mx_square(LdotH));
    float refL = mx_fresnel_schlick(NdotL, 1.0, F90);
    float refV = mx_fresnel_schlick(NdotV, 1.0, F90);
    return refL * refV;
}

// Compute the directional albedo component of Burley diffuse for the given
// view angle and roughness.  Curve fit provided by Stephen Hill.
float mx_burley_diffuse_directional_albedo(float NdotV, float roughness)
{
    float x = NdotV;
    float fit0 = 0.97619 - 0.488095 * mx_pow5(1 - x);
    float fit1 = 1.55754 + (-2.02221 + (2.56283 - 1.06244 * x) * x) * x;
    return mix(fit0, fit1, roughness);
}
