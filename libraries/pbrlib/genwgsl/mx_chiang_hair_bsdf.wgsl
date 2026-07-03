#include "lib/mx_closure_type.wgsl"
#include "lib/mx_microfacet_specular.wgsl"

// https://eugenedeon.com/pdfs/egsrhair.pdf
fn mx_deon_hair_absorption_from_melanin(
    melanin_concentration: f32,
    melanin_redness: f32,
    // constants converted to color via exp(-c). the defaults are lin_rec709 colors, they may be
    // transformed to scene-linear rendering color space.
    eumelanin_color: vec3f,    // default: (0.657704, 0.498077, 0.254106) == exp(-(0.419, 0.697, 1.37))
    pheomelanin_color: vec3f,  // default: (0.829443, 0.670320, 0.349937) == exp(-(0.187, 0.4, 1.05))
    absorption: ptr<function, vec3f>)
{
    var melanin: f32 = -log(max(1.0 - melanin_concentration, 0.0001));
    var eumelanin: f32 = melanin * (1.0 - melanin_redness);
    var pheomelanin: f32 = melanin * melanin_redness;
    *absorption = max(
        eumelanin * -log(eumelanin_color) + pheomelanin * -log(pheomelanin_color), 
        vec3f(0.0)
    );
}

// https://media.disneyanimation.com/uploads/production/publication_asset/152/asset/eurographics2016Fur_Smaller.pdf
fn mx_chiang_hair_absorption_from_color(color: vec3f, betaN: f32, absorption: ptr<function, vec3f>)
{
    var b2: f32 = betaN* betaN;
    var b4: f32 = b2 * b2;
    var b_fac: f32 = 
        5.969 - 
        (0.215 * betaN) + 
        (2.532 * b2) -
        (10.73 * b2 * betaN) + 
        (5.574 * b4) +
        (0.245 * b4 * betaN);
    var sigma: vec3f = log(min(max(color, 0.001), vec3f(1.0))) / b_fac;
    *absorption = sigma * sigma;
}

fn mx_chiang_hair_roughness(
    longitudinal: f32,
    azimuthal: f32,
    scale_TT: f32,   // empirical roughness scale from Marschner et al. (2003).
    scale_TRT: f32,  // default: scale_TT = 0.5, scale_TRT = 2.0
    roughness_R: ptr<function, vec2f>,
    roughness_TT: ptr<function, vec2f>,
    roughness_TRT: ptr<function, vec2f>)
{
    var lr: f32 = clamp(longitudinal, 0.001, 1.0);
    var ar: f32 = clamp(azimuthal, 0.001, 1.0);

    // longitudinal variance
    var v: f32 = 0.726 * lr + 0.812 * lr * lr + 3.7 * pow(lr, 20.0);
    v = v * v;

    var s: f32 = 0.265 * ar + 1.194 * ar * ar + 5.372 * pow(ar, 22.0);

    *roughness_R = vec2f(v, s);
    *roughness_TT = vec2f(v * scale_TT * scale_TT, s);
    *roughness_TRT = vec2f(v * scale_TRT * scale_TRT, s);
}

fn mx_hair_transform_sin_cos(x: f32) -> f32
{
    return sqrt(max(1.0 - x * x, 0.0));
}

fn mx_hair_I0(x: f32) -> f32
{
    var v: f32 = 1.0;
    var n: f32 = 1.0;
    var d: f32 = 1.0;
    var f: f32 = 1.0;
    var x2: f32 = x * x;
    for (var i: i32 = 0; i < 9 ; i++) 
    {
        d *= 4.0 * (f * f);
        n *= x2;
        v += n / d;
        f += 1.0;
    }
    return v;
}

fn mx_hair_log_I0(x: f32) -> f32
{
    if (x > 12.0)
        return x + 0.5 * (-log(2.0 * M_PI) + log(1.0 / x) + 1.0 / (8.0 * x));
    else
        return log(mx_hair_I0(x));
}

fn mx_hair_logistic(x: f32, s: f32) -> f32
{
    var x_val: f32 = x;
    if (x_val > 0.0)
        x_val = -x_val;
    var f: f32 = exp(x_val / s);
    return f / (s * (1.0 + f) * (1.0 + f));
}

fn mx_hair_logistic_cdf(x: f32, s: f32) -> f32
{
    return 1.0 / (1.0 + exp(-x / s));
}

fn mx_hair_trimmed_logistic(x: f32, s: f32, a: f32, b: f32) -> f32
{
    // the constant can be found in Chiang et al. (2016) Appendix A, eq. (12)
    var s_val: f32 = s * 0.626657;  // sqrt(M_PI/8)
    return mx_hair_logistic(x, s_val) / (mx_hair_logistic_cdf(b, s_val) - mx_hair_logistic_cdf(a, s_val));
}

fn mx_hair_phi(p: i32, gammaO: f32, gammaT: f32) -> f32
{
    var fP: f32 = f32(p);
    return 2.0 * fP * gammaT - 2.0 * gammaO + fP * M_PI;
}

fn mx_hair_longitudinal_scattering(  // Mp
    sinThetaI: f32,
    cosThetaI: f32,
    sinThetaO: f32,
    cosThetaO: f32,
    v: f32) -> f32
{
    var inv_v: f32 = 1.0 / v;
    var a: f32 = cosThetaO * cosThetaI * inv_v;
    var b: f32 = sinThetaO * sinThetaI * inv_v;
    if (v < 0.1)
        return exp(mx_hair_log_I0(a) - b - inv_v + 0.6931 + log(0.5 * inv_v));
    else
        return ((exp(-b) * mx_hair_I0(a)) / (2.0 * v * sinh(inv_v)));
}

fn mx_hair_azimuthal_scattering(  // Np
    phi: f32,
    p: i32,
    s: f32,
    gammaO: f32,
    gammaT: f32) -> f32
{
    if (p >= 3)
        return 0.5 / M_PI;

    var dphi: f32 = phi - mx_hair_phi(p, gammaO, gammaT);
    if (isinf(dphi))
        return 0.5 / M_PI;

    while (dphi > M_PI)    dphi -= (2.0 * M_PI);
    while (dphi < (-M_PI)) dphi += (2.0 * M_PI);

    return mx_hair_trimmed_logistic(dphi, s, -M_PI, M_PI);
}

fn mx_hair_alpha_angles(
    alpha: f32,
    sinThetaI: f32,
    cosThetaI: f32,
    angles: ptr<function, array<vec2f, 4>>)
{
    // 0:R, 1:TT, 2:TRT, 3:TRRT+
    for (var i: i32 = 0; i <= 3; i++)
    {
        if (alpha == 0.0 || i == 3)
            (*angles)[i] = vec2f(sinThetaI, cosThetaI);
        else
        {
            var m: f32 = 2.0 - f32(i) * 3.0;
            var sa: f32 = sin(m * alpha);
            var ca: f32 = cos(m * alpha);
            (*angles)[i].x = sinThetaI * ca + cosThetaI * sa;
            (*angles)[i].y = cosThetaI * ca - sinThetaI * sa;
        }
    }
}

fn mx_hair_attenuation(f: f32, T: vec3f, Ap: ptr<function, array<vec3f, 4>>)  // Ap
{
    // 0:R, 1:TT, 2:TRT, 3:TRRT+
    (*Ap)[0] = vec3f(f);
    (*Ap)[1] = (1.0 - f) * (1.0 - f) * T;
    (*Ap)[2] = (*Ap)[1] * T * f;
    (*Ap)[3] = (*Ap)[2] * T * f / (vec3f(1.0) - T * f);
}

fn mx_chiang_hair_bsdf(closureData: ClosureData, tint_R: vec3f, tint_TT: vec3f, tint_TRT: vec3f, ior: f32,
                         roughness_R: vec2f, roughness_TT: vec2f, roughness_TRT: vec2f, cuticle_angle: f32,
                         absorption_coefficient: vec3f, N: vec3f, X: vec3f, bsdf: ptr<function, BSDF>)
{
    var V: vec3f = closureData.V;
    var L: vec3f = closureData.L;
    var N_: vec3f = mx_forward_facing_normal(N, V);  // mutable copy (WGSL params are immutable)
    var X_: vec3f = X;

    (*bsdf).throughput = vec3f(0.0);

    if (closureData.closureType == CLOSURE_TYPE_REFLECTION)
    {
        X_ = normalize(X_ - dot(X_, N_) * N_);
        var Y: vec3f = cross(N_, X_);

        var sinThetaO: f32 = dot(V, X_);
        var sinThetaI: f32 = dot(L, X_);
        var cosThetaO: f32 = mx_hair_transform_sin_cos(sinThetaO);
        var cosThetaI: f32 = mx_hair_transform_sin_cos(sinThetaI);

        var y1: f32 = dot(L, N_);
        var x1: f32 = dot(L, Y);
        var y2: f32 = dot(V, N_);
        var x2: f32 = dot(V, Y);
        var phi: f32 = atan2(y1 * x2 - y2 * x1, x1 * x2 + y1 * y2);

        var k1_p: vec3f = normalize(V - X_ * dot(V, X_));
        var cosGammaO: f32 = dot(N_, k1_p);
        var sinGammaO: f32 = mx_hair_transform_sin_cos(cosGammaO);
        if (dot(k1_p, Y) > 0.0)
            sinGammaO = -sinGammaO;
        var gammaO: f32 = asin(sinGammaO);

        var sinThetaT: f32 = sinThetaO / ior;
        var cosThetaT: f32 = mx_hair_transform_sin_cos(sinThetaT);
        var etaP: f32 = sqrt(max(ior * ior - sinThetaO * sinThetaO, 0.0)) / max(cosThetaO, M_FLOAT_EPS);
        var sinGammaT: f32 = max(min(sinGammaO / etaP, 1.0), -1.0);
        var cosGammaT: f32 = sqrt(1.0 - sinGammaT * sinGammaT);
        var gammaT: f32 = asin(sinGammaT);

        // attenuation
        var Ap: array<vec3f, 4>;
        var fresnel: f32 = mx_fresnel_dielectric(cosThetaO * cosGammaO, ior);
        var T: vec3f = exp(-absorption_coefficient * (2.0 * cosGammaT / cosThetaT));
        mx_hair_attenuation(fresnel, T, &(Ap));

        // parameters for each lobe
        var angles: array<vec2f, 4>;
        var alpha: f32 = cuticle_angle * M_PI - (M_PI / 2.0);  // remap [0, 1] to [-PI/2, PI/2]
        mx_hair_alpha_angles(alpha, sinThetaI, cosThetaI, &(angles));

        var tint: array<vec3f, 4>;
        tint[0] = tint_R;
        tint[1] = tint_TT;
        tint[2] = tint_TRT;
        tint[3] = tint_TRT;

        var roughness_R_clamped: vec2f = clamp(roughness_R, 0.001, 1.0);
        var roughness_TT_clamped: vec2f = clamp(roughness_TT, 0.001, 1.0);
        var roughness_TRT_clamped: vec2f = clamp(roughness_TRT, 0.001, 1.0);

        var vs: array<vec2f, 4>;
        vs[0] = roughness_R_clamped;
        vs[1] = roughness_TT_clamped;
        vs[2] = roughness_TRT_clamped;
        vs[3] = roughness_TRT_clamped;

        // R, TT, TRT, TRRT+
        var F: vec3f = vec3f(0.0);
        for (var i: i32 = 0; i <= 3; i++)
        {
            tint[i] = max(tint[i], vec3f(0.0));
            var Mp: f32 = mx_hair_longitudinal_scattering(angles[i].x, angles[i].y, sinThetaO, cosThetaO, vs[i].x);
            var Np: f32 = select(mx_hair_azimuthal_scattering(phi, i, vs[i].y, gammaO, gammaT), (1.0 / 2.0 * M_PI), (i == 3));
            F += Mp * Np * tint[i] * Ap[i];
        }

        (*bsdf).response = F * closureData.occlusion * (1.0 / M_PI);
    }
    else if (closureData.closureType == CLOSURE_TYPE_INDIRECT)
    {
        // This indirect term is a *very* rough approximation.

        var NdotV: f32 = clamp(dot(N_, V), M_FLOAT_EPS, 1.0);
        var fd: FresnelData = mx_init_fresnel_dielectric(ior, 0.0, 1.0);
        var F: vec3f = mx_compute_fresnel(NdotV, fd);

        var roughness: vec2f = (roughness_R + roughness_TT + roughness_TRT) / vec2f(3.0);  // ?
        var safeAlpha: vec2f = clamp(roughness, vec2f(M_FLOAT_EPS), vec2f(1.0));
        var avgAlpha: f32 = mx_average_alpha(safeAlpha);

        // Use GGX to match the behavior of mx_environment_radiance.
        var F0: f32 = mx_ior_to_f0(ior);
        var comp: vec3f = mx_ggx_energy_compensation(NdotV, avgAlpha, F);
        var dirAlbedo: vec3f = mx_ggx_dir_albedo(NdotV, avgAlpha, F0, 1.0) * comp;

        var Li: vec3f = mx_environment_radiance(N_, V, X_, safeAlpha, 0, fd);
        var tint: vec3f = (tint_R + tint_TT + tint_TRT) / vec3f(3.0);  // ?

        (*bsdf).response = Li * comp * tint;
    }
}
