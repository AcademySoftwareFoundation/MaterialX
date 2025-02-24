#include "lib/mx_closure_type.glsl"
#include "lib/mx_microfacet_specular.glsl"

// https://eugenedeon.com/pdfs/egsrhair.pdf
void mx_deon_hair_absorption_from_melanin(
    float melanin_concentration,
    float melanin_redness,
    // constants converted to color via exp(-c). the defaults are lin_rec709 colors, they may be
    // transformed to scene-linear rendering color space.
    vec3 eumelanin_color,    // default: (0.657704, 0.498077, 0.254106) == exp(-(0.419, 0.697, 1.37))
    vec3 pheomelanin_color,  // default: (0.829443, 0.670320, 0.349937) == exp(-(0.187, 0.4, 1.05))
    out vec3 absorption)
{
    float melanin = -log(max(1.0 - melanin_concentration, 0.0001));
    float eumelanin = melanin * (1.0 - melanin_redness);
    float pheomelanin = melanin * melanin_redness;
    absorption = max(
        eumelanin * -log(eumelanin_color) + pheomelanin * -log(pheomelanin_color), 
        vec3(0.0)
    );
}

// https://media.disneyanimation.com/uploads/production/publication_asset/152/asset/eurographics2016Fur_Smaller.pdf
void mx_chiang_hair_absorption_from_color(vec3 color, float betaN, out vec3 absorption)
{
    float b2 = betaN* betaN;
    float b4 = b2 * b2;
    float b_fac = 
        5.969 - 
        (0.215 * betaN) + 
        (2.532 * b2) -
        (10.73 * b2 * betaN) + 
        (5.574 * b4) +
        (0.245 * b4 * betaN);
    vec3 sigma = log(min(max(color, 0.001), vec3(1.0))) / b_fac;
    absorption = sigma * sigma;
}

void mx_chiang_hair_roughness(
    float longitudinal,
    float azimuthal,
    float scale_TT,   // empirical roughness scale from Marschner et al. (2003).
    float scale_TRT,  // default: scale_TT = 0.5, scale_TRT = 2.0
    out vec2 roughness_R,
    out vec2 roughness_TT,
    out vec2 roughness_TRT
)
{
    float lr = clamp(longitudinal, 0.001, 1.0);
    float ar = clamp(azimuthal, 0.001, 1.0);

    // longitudinal variance
    float v = 0.726 * lr + 0.812 * lr * lr + 3.7 * pow(lr, 20.0);
    v = v * v;

    float s = 0.265 * ar + 1.194 * ar * ar + 5.372 * pow(ar, 22.0);

    roughness_R = vec2(v, s);
    roughness_TT = vec2(v * scale_TT * scale_TT, s);
    roughness_TRT = vec2(v * scale_TRT * scale_TRT, s);
}

float mx_hair_transform_sin_cos(float x)
{
    return sqrt(max(1.0 - x * x, 0.0));
}

float mx_hair_I0(float x)
{
    float v = 1.0;
    float n = 1.0;
    float d = 1.0;
    float f = 1.0;
    float x2 = x * x;
    for (int i = 0; i < 9 ; ++i)
    {
        d *= 4.0 * (f * f);
        n *= x2;
        v += n / d;
        f += 1.0;
    }
    return v;
}

float mx_hair_log_I0(float x)
{
    if (x > 12.0)
        return x + 0.5 * (-log(2.0 * M_PI) + log(1.0 / x) + 1.0 / (8.0 * x));
    else
        return log(mx_hair_I0(x));
}

float mx_hair_logistic(float x, float s)
{
    if (x > 0.0)
        x = -x;
    float f = exp(x / s);
    return f / (s * (1.0 + f) * (1.0 + f));
}

float mx_hair_logistic_cdf(float x, float s)
{
    return 1.0 / (1.0 + exp(-x / s));
}

float mx_hair_trimmed_logistic(float x, float s, float a, float b)
{
    // the constant can be found in Chiang et al. (2016) Appendix A, eq. (12)
    s *= 0.626657;  // sqrt(M_PI/8)
    return mx_hair_logistic(x, s) / (mx_hair_logistic_cdf(b, s) - mx_hair_logistic_cdf(a, s));
}

float mx_hair_phi(int p, float gammaO, float gammaT)
{
    float fP = float(p);
    return 2.0 * fP * gammaT - 2.0 * gammaO + fP * M_PI;
}

float mx_hair_longitudinal_scattering(  // Mp
    float sinThetaI,
    float cosThetaI,
    float sinThetaO,
    float cosThetaO,
    float v
)
{
    float inv_v = 1.0 / v;
    float a = cosThetaO * cosThetaI * inv_v;
    float b = sinThetaO * sinThetaI * inv_v;
    if (v < 0.1)
        return exp(mx_hair_log_I0(a) - b - inv_v + 0.6931 + log(0.5 * inv_v));
    else
        return ((exp(-b) * mx_hair_I0(a)) / (2.0 * v * sinh(inv_v)));
}

float mx_hair_azimuthal_scattering(  // Np
    float phi,
    int p,
    float s,
    float gammaO,
    float gammaT
)
{
    if (p >= 3)
        return float(0.5 / M_PI);

    float dphi = phi - mx_hair_phi(p, gammaO, gammaT);
    if (isinf(dphi))
        return float(0.5 / M_PI);

    while (dphi > M_PI)    dphi -= (2.0 * M_PI);
    while (dphi < (-M_PI)) dphi += (2.0 * M_PI);

    return mx_hair_trimmed_logistic(dphi, s, -M_PI, M_PI);
}

void mx_hair_alpha_angles(
    float alpha,
    float sinThetaI,
    float cosThetaI,
    out vec2 angles[4]
)
{
    // 0:R, 1:TT, 2:TRT, 3:TRRT+
    for (int i = 0; i <= 3; ++i)
    {
        if (alpha == 0.0 || i == 3)
            angles[i] = vec2(sinThetaI, cosThetaI);
        else
        {
            float m = 2.0 - float(i) * 3.0;
            float sa = sin(m * alpha);
            float ca = cos(m * alpha);
            angles[i].x = sinThetaI * ca + cosThetaI * sa;
            angles[i].y = cosThetaI * ca - sinThetaI * sa;
        }
    }
}

void mx_hair_attenuation(float f, vec3 T, out vec3 Ap[4])  // Ap
{
    // 0:R, 1:TT, 2:TRT, 3:TRRT+
    Ap[0] = vec3(f);
    Ap[1] = (1.0 - f) * (1.0 - f) * T;
    Ap[2] = Ap[1] * T * f;
    Ap[3] = Ap[2] * T * f / (vec3(1.0) - T * f);
}

void mx_chiang_hair_bsdf(ClosureData closureData, vec3 tint_R, vec3 tint_TT, vec3 tint_TRT, float ior,
                         vec2 roughness_R, vec2 roughness_TT, vec2 roughness_TRT, float cuticle_angle,
                         vec3 absorption_coefficient, vec3 N, vec3 X, inout BSDF bsdf)
{
    vec3 V = closureData.V;
    vec3 L = closureData.L;

    N = mx_forward_facing_normal(N, V);

    bsdf.throughput = vec3(0.0);

    if (closureData.closureType == CLOSURE_TYPE_REFLECTION)
    {
        X = normalize(X - dot(X, N) * N);
        vec3 Y = cross(N, X);

        float sinThetaO = dot(V, X);
        float sinThetaI = dot(L, X);
        float cosThetaO = mx_hair_transform_sin_cos(sinThetaO);
        float cosThetaI = mx_hair_transform_sin_cos(sinThetaI);

        float y1 = dot(L, N);
        float x1 = dot(L, Y);
        float y2 = dot(V, N);
        float x2 = dot(V, Y);
        float phi = mx_atan(y1 * x2 - y2 * x1, x1 * x2 + y1 * y2);

        vec3 k1_p = normalize(V - X * dot(V, X));
        float cosGammaO = dot(N, k1_p);
        float sinGammaO = mx_hair_transform_sin_cos(cosGammaO);
        if (dot(k1_p, Y) > 0.0)
            sinGammaO = -sinGammaO;
        float gammaO = asin(sinGammaO);

        float sinThetaT = sinThetaO / ior;
        float cosThetaT = mx_hair_transform_sin_cos(sinThetaT);
        float etaP = sqrt(max(ior * ior - sinThetaO * sinThetaO, 0.0)) / max(cosThetaO, M_FLOAT_EPS);
        float sinGammaT = max(min(sinGammaO / etaP, 1.0), -1.0);
        float cosGammaT = sqrt(1.0 - sinGammaT * sinGammaT);
        float gammaT = asin(sinGammaT);

        // attenuation
        vec3 Ap[4];
        float fresnel = mx_fresnel_dielectric(cosThetaO * cosGammaO, ior);
        vec3 T = exp(-absorption_coefficient * (2.0 * cosGammaT / cosThetaT));
        mx_hair_attenuation(fresnel, T, Ap);

        // parameters for each lobe
        vec2 angles[4];
        float alpha = cuticle_angle * M_PI - (M_PI / 2.0);  // remap [0, 1] to [-PI/2, PI/2]
        mx_hair_alpha_angles(alpha, sinThetaI, cosThetaI, angles);

        vec3 tint[4];
        tint[0] = tint_R;
        tint[1] = tint_TT;
        tint[2] = tint_TRT;
        tint[3] = tint_TRT;

        roughness_R = clamp(roughness_R, 0.001, 1.0);
        roughness_TT = clamp(roughness_TT, 0.001, 1.0);
        roughness_TRT = clamp(roughness_TRT, 0.001, 1.0);

        vec2 vs[4];
        vs[0] = roughness_R;
        vs[1] = roughness_TT;
        vs[2] = roughness_TRT;
        vs[3] = roughness_TRT;

        // R, TT, TRT, TRRT+
        vec3 F = vec3(0.0);
        for (int i = 0; i <= 3; ++i)
        {
            tint[i] = max(tint[i], vec3(0.0));
            float Mp = mx_hair_longitudinal_scattering(angles[i].x, angles[i].y, sinThetaO, cosThetaO, vs[i].x);
            float Np = (i == 3) ?  (1.0 / 2.0 * M_PI) : mx_hair_azimuthal_scattering(phi, i, vs[i].y, gammaO, gammaT);
            F += Mp * Np * tint[i] * Ap[i];
        }

        bsdf.response = F * closureData.occlusion * M_PI_INV;
    }
    else if (closureData.closureType == CLOSURE_TYPE_INDIRECT)
    {
        // This indirect term is a *very* rough approximation.

        float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);
        FresnelData fd = mx_init_fresnel_dielectric(ior, 0.0, 1.0);
        vec3 F = mx_compute_fresnel(NdotV, fd);

        vec2 roughness = (roughness_R + roughness_TT + roughness_TRT) / vec2(3.0);  // ?
        vec2 safeAlpha = clamp(roughness, M_FLOAT_EPS, 1.0);
        float avgAlpha = mx_average_alpha(safeAlpha);

        // Use GGX to match the behavior of mx_environment_radiance.
        float F0 = mx_ior_to_f0(ior);
        vec3 comp = mx_ggx_energy_compensation(NdotV, avgAlpha, F);
        vec3 dirAlbedo = mx_ggx_dir_albedo(NdotV, avgAlpha, F0, 1.0) * comp;

        vec3 Li = mx_environment_radiance(N, V, X, safeAlpha, 0, fd);
        vec3 tint = (tint_R + tint_TT + tint_TRT) / vec3(3.0);  // ?

        bsdf.response = Li * comp * tint;
    }
}
