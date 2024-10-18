#include "mx_microfacet.glsl"

// https://fpsunflower.github.io/ckulla/data/s2017_pbs_imageworks_sheen.pdf
// Equation 2
float mx_imageworks_sheen_NDF(float NdotH, float roughness)
{
    float invRoughness = 1.0 / max(roughness, 0.005);
    float cos2 = NdotH * NdotH;
    float sin2 = 1.0 - cos2;
    return (2.0 + invRoughness) * pow(sin2, invRoughness * 0.5) / (2.0 * M_PI);
}

float mx_imageworks_sheen_brdf(float NdotL, float NdotV, float NdotH, float roughness)
{
    // Microfacet distribution.
    float D = mx_imageworks_sheen_NDF(NdotH, roughness);

    // Fresnel and geometry terms are ignored.
    float F = 1.0;
    float G = 1.0;

    // We use a smoother denominator, as in:
    // https://blog.selfshadow.com/publications/s2013-shading-course/rad/s2013_pbs_rad_notes.pdf
    return D * F * G / (4.0 * (NdotL + NdotV - NdotL*NdotV));
}

// Rational quadratic fit to Monte Carlo data for Imageworks sheen directional albedo.
float mx_imageworks_sheen_dir_albedo_analytic(float NdotV, float roughness)
{
    vec2 r = vec2(13.67300, 1.0) +
             vec2(-68.78018, 61.57746) * NdotV +
             vec2(799.08825, 442.78211) * roughness +
             vec2(-905.00061, 2597.49308) * NdotV * roughness +
             vec2(60.28956, 121.81241) * mx_square(NdotV) +
             vec2(1086.96473, 3045.55075) * mx_square(roughness);
    return r.x / r.y;
}

float mx_imageworks_sheen_dir_albedo_table_lookup(float NdotV, float roughness)
{
#if DIRECTIONAL_ALBEDO_METHOD == 1
    if (textureSize($albedoTable, 0).x > 1)
    {
        return texture($albedoTable, vec2(NdotV, roughness)).b;
    }
#endif
    return 0.0;
}

float mx_imageworks_sheen_dir_albedo_monte_carlo(float NdotV, float roughness)
{
    NdotV = clamp(NdotV, M_FLOAT_EPS, 1.0);
    vec3 V = vec3(sqrt(1.0f - mx_square(NdotV)), 0, NdotV);

    float radiance = 0.0;
    const int SAMPLE_COUNT = 64;
    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
        vec2 Xi = mx_spherical_fibonacci(i, SAMPLE_COUNT);

        // Compute the incoming light direction and half vector.
        vec3 L = mx_uniform_sample_hemisphere(Xi);
        vec3 H = normalize(L + V);
        
        // Compute dot products for this sample.
        float NdotL = clamp(L.z, M_FLOAT_EPS, 1.0);
        float NdotH = clamp(H.z, M_FLOAT_EPS, 1.0);

        // Compute sheen reflectance.
        float reflectance = mx_imageworks_sheen_brdf(NdotL, NdotV, NdotH, roughness);

        // Add the radiance contribution of this sample.
        //   uniform_pdf = 1 / (2 * PI)
        //   radiance = reflectance * NdotL / uniform_pdf;
        radiance += reflectance * NdotL * 2.0 * M_PI;
    }

    // Return the final directional albedo.
    return radiance / float(SAMPLE_COUNT);
}

float mx_imageworks_sheen_dir_albedo(float NdotV, float roughness)
{
#if DIRECTIONAL_ALBEDO_METHOD == 0
    float dirAlbedo = mx_imageworks_sheen_dir_albedo_analytic(NdotV, roughness);
#elif DIRECTIONAL_ALBEDO_METHOD == 1
    float dirAlbedo = mx_imageworks_sheen_dir_albedo_table_lookup(NdotV, roughness);
#else
    float dirAlbedo = mx_imageworks_sheen_dir_albedo_monte_carlo(NdotV, roughness);
#endif
    return clamp(dirAlbedo, 0.0, 1.0);
}

// The following functions are adapted from https://github.com/tizian/ltc-sheen.
// "Practical Multiple-Scattering Sheen Using Linearly Transformed Cosines", Zeltner et al.

// Gaussian fit to directional albedo table.
float mx_zeltner_sheen_dir_albedo(float x, float y)
{
    float s = y*(0.0206607 + 1.58491*y)/(0.0379424 + y*(1.32227 + y));
    float m = y*(-0.193854 + y*(-1.14885 + y*(1.7932 - 0.95943*y*y)))/(0.046391 + y);
    float o = y*(0.000654023 + (-0.0207818 + 0.119681*y)*y)/(1.26264 + y*(-1.92021 + y));
    return exp(-0.5*mx_square((x - m)/s))/(s*sqrt(2.0*M_PI)) + o;
}

// Rational fits to LTC matrix coefficients.
float mx_zeltner_sheen_ltc_aInv(float x, float y)
{
    return (2.58126*x + 0.813703*y)*y/(1.0 + 0.310327*x*x + 2.60994*x*y);
}

float mx_zeltner_sheen_ltc_bInv(float x, float y)
{
    return sqrt(1.0 - x)*(y - 1.0)*y*y*y/(0.0000254053 + 1.71228*x - 1.71506*x*y + 1.34174*y*y);
}

// V and N are assumed to be unit vectors.
mat3 mx_orthonormal_basis_ltc(vec3 V, vec3 N, float NdotV)
{
    // Generate a tangent vector in the plane of V and N.
    // This required to correctly orient the LTC lobe.
    vec3 X = V - N*NdotV;
    float lenSqr = dot(X, X);
    if (lenSqr > 0.0)
    {
        X *= mx_inversesqrt(lenSqr);
        vec3 Y = cross(N, X);
        return mat3(X, Y, N);
    }

    // If lenSqr == 0, then V == N, so any orthonormal basis will do.
    return mx_orthonormal_basis(N);
}

// Multiplication by directional albedo is handled by the calling function.
float mx_zeltner_sheen_brdf(vec3 L, vec3 V, vec3 N, float NdotV, float roughness)
{
    mat3 toLTC = transpose(mx_orthonormal_basis_ltc(V, N, NdotV));
    vec3 w = toLTC * L;

    float aInv = mx_zeltner_sheen_ltc_aInv(NdotV, roughness);
    float bInv = mx_zeltner_sheen_ltc_bInv(NdotV, roughness);

    // Transform w to original configuration (clamped cosine).
    //                 |aInv    0 bInv|
    // wo = M^-1 . w = |   0 aInv    0| . w
    //                 |   0    0    1|
    vec3 wo = vec3(aInv*w.x + bInv*w.z, aInv * w.y, w.z);
    float lenSqr = dot(wo, wo);

    // D(w) = Do(M^-1.w / ||M^-1.w||) . |M^-1| / ||M^-1.w||^3
    //      = Do(M^-1.w) . |M^-1| / ||M^-1.w||^4
    //      = Do(wo) . |M^-1| / dot(wo, wo)^2
    //      = Do(wo) . aInv^2 / dot(wo, wo)^2
    //      = Do(wo) . (aInv / dot(wo, wo))^2
    return max(wo.z, 0.0) * M_PI_INV * mx_square(aInv / lenSqr);
}

vec3 mx_zeltner_sheen_importance_sample(vec2 Xi, vec3 V, vec3 N, float roughness, out float pdf)
{
    float NdotV = clamp(dot(N, V), 0.0, 1.0);
    roughness = clamp(roughness, 0.01, 1.0); // Clamp to range of original impl.

    vec3 wo = mx_cosine_sample_hemisphere(Xi);

    float aInv = mx_zeltner_sheen_ltc_aInv(NdotV, roughness);
    float bInv = mx_zeltner_sheen_ltc_bInv(NdotV, roughness);

    // Transform wo from original configuration (clamped cosine).
    //              |1/aInv      0 -bInv/aInv|
    // w = M . wo = |     0 1/aInv          0| . wo
    //              |     0      0          1|    
    vec3 w = vec3(wo.x/aInv - wo.z*bInv/aInv, wo.y / aInv, wo.z);

    float lenSqr = dot(w, w);
    w *= mx_inversesqrt(lenSqr);

    // D(w) = Do(wo) . ||M.wo||^3 / |M|
    //      = Do(wo / ||M.wo||) . ||M.wo||^4 / |M| 
    //      = Do(w) . ||M.wo||^4 / |M| (possible because M doesn't change z component)
    //      = Do(w) . dot(w, w)^2 * aInv^2
    //      = Do(w) . (aInv * dot(w, w))^2
    pdf = max(w.z, 0.0) * M_PI_INV * mx_square(aInv * lenSqr);

    mat3 fromLTC = mx_orthonormal_basis_ltc(V, N, NdotV);
    w = fromLTC * w;

    return w;
}
