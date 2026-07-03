#include "mx_microfacet.wgsl"

fn mx_average_alpha(alpha: vec2f) -> f32 {
    return sqrt(alpha.x * alpha.y);
}

// Convert IOR to F0 (normal-incidence reflectivity)
fn mx_ior_to_f0(ior: f32) -> f32 {
    return mx_square_f32((ior - 1f) / (ior + 1f));
}

// Convert F0 to IOR (for transmission)
fn mx_f0_to_ior(F0: f32) -> f32 {
    let sqrtF0 = sqrt(clamp(F0, 0.01, 0.99));
    return (1.0 + sqrtF0) / (1.0 - sqrtF0);
}

fn mx_f0_to_ior_vec3(F0: vec3f) -> vec3f {
    let sqrtF0 = sqrt(clamp(F0, vec3f(0.01), vec3f(0.99)));
    return (vec3f(1.0) + sqrtF0) / (vec3f(1.0) - sqrtF0);
}

// Fresnel models, mirroring mx_microfacet_specular.glsl.
const FRESNEL_MODEL_DIELECTRIC: i32 = 0;
const FRESNEL_MODEL_CONDUCTOR: i32 = 1;
const FRESNEL_MODEL_SCHLICK: i32 = 2;

// Number of terms in the thin-film Airy series (generator option hwAiryFresnelIterations; default 2).
const AIRY_FRESNEL_ITERATIONS: i32 = 2;

fn mx_init_fresnel_dielectric(ior: f32, thinfilm_thickness: f32, thinfilm_ior: f32) -> FresnelData {
    var fd: FresnelData;
    fd.model = FRESNEL_MODEL_DIELECTRIC;
    fd.airy = thinfilm_thickness > 0.0;
    let F0_scalar = mx_ior_to_f0(ior);
    fd.F0 = vec3f(F0_scalar);
    fd.F82 = vec3f(F0_scalar);
    fd.F90 = vec3f(1f);
    fd.exponent = 5f;
    fd.thinfilm_thickness = thinfilm_thickness;
    fd.thinfilm_ior = thinfilm_ior;
    fd.ior = vec3f(ior);
    fd.extinction = vec3f(0.0);
    return fd;
}

// Initialize FresnelData for a conductor material.
// Computes F0 from complex IOR (n, k) using the exact Fresnel equation at normal incidence:
//   F0 = ((n - 1)^2 + k^2) / ((n + 1)^2 + k^2)
fn mx_init_fresnel_conductor(ior: vec3f, extinction: vec3f, thinfilm_thickness: f32, thinfilm_ior: f32) -> FresnelData {
    var fd: FresnelData;
    fd.model = FRESNEL_MODEL_CONDUCTOR;
    fd.airy = thinfilm_thickness > 0.0;
    fd.ior = ior;
    fd.extinction = extinction;
    let n_minus_1 = ior - vec3f(1.0);
    let n_plus_1 = ior + vec3f(1.0);
    let k2 = extinction * extinction;
    fd.F0 = (n_minus_1 * n_minus_1 + k2) / (n_plus_1 * n_plus_1 + k2);
    fd.F82 = fd.F0;
    fd.F90 = vec3f(1.0);
    fd.exponent = 5.0;
    fd.thinfilm_thickness = thinfilm_thickness;
    fd.thinfilm_ior = thinfilm_ior;
    return fd;
}

fn mx_saturate(x: f32) -> f32 {
    return clamp(x, 0.0, 1.0);
}

fn mx_saturate_v3(x: vec3f) -> vec3f {
    return clamp(x, vec3f(0.0), vec3f(1.0));
}

// Initialize Fresnel data for generalized Schlick model.
fn mx_init_fresnel_schlick(
    color0: vec3f,
    color82: vec3f,
    color90: vec3f,
    exponent: f32,
    thinfilm_thickness: f32,
    thinfilm_ior: f32
) -> FresnelData {
    var fd: FresnelData;
    fd.model = FRESNEL_MODEL_SCHLICK;
    fd.airy = thinfilm_thickness > 0.0;
    fd.F0 = mx_saturate_v3(color0);
    fd.F82 = mx_saturate_v3(color82);
    fd.F90 = mx_saturate_v3(color90);
    fd.exponent = exponent;
    fd.thinfilm_thickness = thinfilm_thickness;
    fd.thinfilm_ior = thinfilm_ior;
    return fd;
}

// https://renderwonk.com/publications/wp-generalization-adobe/gen-adobe.pdf
fn mx_fresnel_hoffman_schlick(cosTheta: f32, fd: FresnelData) -> vec3f {
    let COS_THETA_MAX: f32 = 1.0 / 7.0;
    let COS_THETA_FACTOR: f32 = 1.0 / (COS_THETA_MAX * pow(1.0 - COS_THETA_MAX, 6.0));

    let x = clamp(cosTheta, 0.0, 1.0);
    let a = mix(fd.F0, fd.F90, pow(1.0 - COS_THETA_MAX, fd.exponent)) * (vec3f(1.0) - fd.F82) * COS_THETA_FACTOR;
    return mix(fd.F0, fd.F90, pow(1.0 - x, fd.exponent)) - a * x * mx_pow6(1.0 - x);
}

// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
fn mx_fresnel_dielectric(cosTheta: f32, ior: f32) -> f32 {
    let c = cosTheta;
    let g2 = ior * ior + c * c - 1.0;
    if (g2 < 0.0) {
        // Total internal reflection
        return 1.0;
    }
    let g = sqrt(g2);
    return 0.5 * mx_square_f32((g - c) / (g + c)) *
                (1.0 + mx_square_f32(((g + c) * c - 1.0) / ((g - c) * c + 1.0)));
}

// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
fn mx_fresnel_dielectric_polarized(cosTheta: f32, ior: f32) -> vec2f {
    let cosTheta2 = mx_square_f32(clamp(cosTheta, 0.0, 1.0));
    let sinTheta2 = 1.0 - cosTheta2;

    let t0 = max(ior * ior - sinTheta2, 0.0);
    let t1 = t0 + cosTheta2;
    let t2 = 2.0 * sqrt(t0) * cosTheta;
    let Rs = (t1 - t2) / (t1 + t2);

    let t3 = cosTheta2 * t0 + sinTheta2 * sinTheta2;
    let t4 = t2 * sinTheta2;
    let Rp = Rs * (t3 - t4) / (t3 + t4);

    return vec2f(Rp, Rs);
}

// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
fn mx_fresnel_conductor_polarized(cosTheta: f32, n: vec3f, k: vec3f, Rp: ptr<function, vec3f>, Rs: ptr<function, vec3f>) {
    let cosTheta2 = mx_square_f32(clamp(cosTheta, 0.0, 1.0));
    let sinTheta2 = 1.0 - cosTheta2;
    let n2 = n * n;
    let k2 = k * k;

    let t0 = n2 - k2 - vec3f(sinTheta2);
    let a2plusb2 = sqrt(t0 * t0 + 4.0 * n2 * k2);
    let t1 = a2plusb2 + vec3f(cosTheta2);
    let a = sqrt(max(0.5 * (a2plusb2 + t0), vec3f(0.0)));
    let t2 = 2.0 * a * cosTheta;
    (*Rs) = (t1 - t2) / (t1 + t2);

    let t3 = cosTheta2 * a2plusb2 + vec3f(sinTheta2 * sinTheta2);
    let t4 = t2 * sinTheta2;
    (*Rp) = (*Rs) * (t3 - t4) / (t3 + t4);
}

fn mx_fresnel_conductor(cosTheta: f32, n: vec3f, k: vec3f) -> vec3f {
    var Rp: vec3f;
    var Rs: vec3f;
    mx_fresnel_conductor_polarized(cosTheta, n, k, &Rp, &Rs);
    return 0.5 * (Rp + Rs);
}

// https://belcour.github.io/blog/research/publication/2017/05/01/brdf-thin-film.html
fn mx_fresnel_conductor_phase_polarized(cosTheta: f32, eta1: f32, eta2: vec3f, kappa2: vec3f, phiP: ptr<function, vec3f>, phiS: ptr<function, vec3f>) {
    let k2 = kappa2 / eta2;
    let sinThetaSqr = vec3f(1.0 - cosTheta * cosTheta);
    let A = eta2 * eta2 * (vec3f(1.0) - k2 * k2) - eta1 * eta1 * sinThetaSqr;
    let B = sqrt(A * A + mx_square_vec3(2.0 * eta2 * eta2 * k2));
    let U = sqrt((A + B) / 2.0);
    let V = max(vec3f(0.0), sqrt((B - A) / 2.0));

    (*phiS) = atan2(2.0 * eta1 * V * cosTheta, U * U + V * V - vec3f(mx_square_f32(eta1 * cosTheta)));
    (*phiP) = atan2(2.0 * eta1 * eta2 * eta2 * cosTheta * (2.0 * k2 * U - (vec3f(1.0) - k2 * k2) * V),
                    mx_square_vec3(eta2 * eta2 * (vec3f(1.0) + k2 * k2) * cosTheta) - eta1 * eta1 * (U * U + V * V));
}

// https://belcour.github.io/blog/research/publication/2017/05/01/brdf-thin-film.html
fn mx_eval_sensitivity(opd: f32, shift: vec3f) -> vec3f {
    // Use Gaussian fits, given by 3 parameters: val, pos and var.
    let phase = 2.0 * M_PI * opd;
    let val = vec3f(5.4856e-13, 4.4201e-13, 5.2481e-13);
    let pos = vec3f(1.6810e+06, 1.7953e+06, 2.2084e+06);
    let vari = vec3f(4.3278e+09, 9.3046e+09, 6.6121e+09);
    var xyz = val * sqrt(2.0 * M_PI * vari) * cos(pos * phase + shift) * exp(-vari * phase * phase);
    xyz.x = xyz.x + 9.7470e-14 * sqrt(2.0 * M_PI * 4.5282e+09) * cos(2.2399e+06 * phase + shift.x) * exp(-4.5282e+09 * phase * phase);
    return xyz / 1.0685e-7;
}

// A Practical Extension to Microfacet Theory for the Modeling of Varying Iridescence
// https://belcour.github.io/blog/research/publication/2017/05/01/brdf-thin-film.html
fn mx_fresnel_airy(cosTheta: f32, fd: FresnelData) -> vec3f {
    // XYZ to CIE 1931 RGB color space (using neutral E illuminant).
    let XYZ_TO_RGB = mat3x3f(2.3706743, -0.5138850, 0.0052982, -0.9000405, 1.4253036, -0.0146949, -0.4706338, 0.0885814, 1.0093968);

    // Assume vacuum on the outside.
    let eta1 = 1.0;
    let eta2 = max(fd.thinfilm_ior, eta1);
    let eta3 = select(fd.ior, mx_f0_to_ior_vec3(fd.F0), fd.model == FRESNEL_MODEL_SCHLICK);
    let kappa3 = select(fd.extinction, vec3f(0.0), fd.model == FRESNEL_MODEL_SCHLICK);
    let cosThetaT = sqrt(1.0 - (1.0 - mx_square_f32(cosTheta)) * mx_square_f32(eta1 / eta2));

    // First interface.
    var R12 = mx_fresnel_dielectric_polarized(cosTheta, eta2 / eta1);
    if (cosThetaT <= 0.0) {
        // Total internal reflection
        R12 = vec2f(1.0);
    }
    let T121 = vec2f(1.0) - R12;

    // Second interface.
    var R23p: vec3f;
    var R23s: vec3f;
    if (fd.model == FRESNEL_MODEL_SCHLICK) {
        let f = mx_fresnel_hoffman_schlick(cosThetaT, fd);
        R23p = 0.5 * f;
        R23s = 0.5 * f;
    } else {
        mx_fresnel_conductor_polarized(cosThetaT, eta3 / eta2, kappa3 / eta2, &R23p, &R23s);
    }

    // Phase shift.
    let cosB = cos(atan(eta2 / eta1));
    let phi21 = vec2f(select(M_PI, 0.0, cosTheta < cosB), M_PI);
    var phi23p: vec3f;
    var phi23s: vec3f;
    if (fd.model == FRESNEL_MODEL_SCHLICK) {
        phi23p = vec3f(select(0.0, M_PI, eta3.x < eta2),
                       select(0.0, M_PI, eta3.y < eta2),
                       select(0.0, M_PI, eta3.z < eta2));
        phi23s = phi23p;
    } else {
        mx_fresnel_conductor_phase_polarized(cosThetaT, eta2, eta3, kappa3, &phi23p, &phi23s);
    }
    let r123p = max(sqrt(R12.x * R23p), vec3f(0.0));
    let r123s = max(sqrt(R12.y * R23s), vec3f(0.0));

    // Iridescence term.
    var I = vec3f(0.0);
    var Cm: vec3f;
    var Sm: vec3f;

    // Optical path difference.
    let distMeters = fd.thinfilm_thickness * 1.0e-9;
    let opd = 2.0 * eta2 * cosThetaT * distMeters;

    // Iridescence term using spectral antialiasing for parallel polarization.
    let Rsp = (mx_square_f32(T121.x) * R23p) / (vec3f(1.0) - R12.x * R23p);
    I += vec3f(R12.x) + Rsp;
    Cm = Rsp - vec3f(T121.x);
    for (var m: i32 = 1; m <= AIRY_FRESNEL_ITERATIONS; m++) {
        Cm *= r123p;
        Sm = 2.0 * mx_eval_sensitivity(f32(m) * opd, f32(m) * (phi23p + vec3f(phi21.x)));
        I += Cm * Sm;
    }

    // Iridescence term using spectral antialiasing for perpendicular polarization.
    let Rpp = (mx_square_f32(T121.y) * R23s) / (vec3f(1.0) - R12.y * R23s);
    I += vec3f(R12.y) + Rpp;
    Cm = Rpp - vec3f(T121.y);
    for (var m: i32 = 1; m <= AIRY_FRESNEL_ITERATIONS; m++) {
        Cm *= r123s;
        Sm = 2.0 * mx_eval_sensitivity(f32(m) * opd, f32(m) * (phi23s + vec3f(phi21.y)));
        I += Cm * Sm;
    }

    // Average parallel and perpendicular polarization.
    I *= 0.5;

    // Convert back to RGB reflectance.
    I = clamp(XYZ_TO_RGB * I, vec3f(0.0), vec3f(1.0));

    return I;
}

// Fresnel computation dispatching on the Fresnel model, with thin-film (Airy) iridescence.
fn mx_compute_fresnel(cosTheta: f32, fd: FresnelData) -> vec3f {
    if (fd.airy) {
        return mx_fresnel_airy(cosTheta, fd);
    } else if (fd.model == FRESNEL_MODEL_DIELECTRIC) {
        return vec3f(mx_fresnel_dielectric(cosTheta, fd.ior.x));
    } else if (fd.model == FRESNEL_MODEL_CONDUCTOR) {
        return mx_fresnel_conductor(cosTheta, fd.ior, fd.extinction);
    }
    return mx_fresnel_hoffman_schlick(cosTheta, fd);
}

// https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf
// Appendix B.2 Equation 13
fn mx_ggx_NDF(H: vec3f, alpha: vec2f) -> f32 {
    let He = H.xy / alpha;
    let denom = dot(He, He) + mx_square_f32(H.z);
    return 1.0 / (M_PI * alpha.x * alpha.y * mx_square_f32(denom));
}

// Height-correlated Smith G2 for GGX.
fn mx_ggx_smith_G2(NdotL: f32, NdotV: f32, alpha: f32) -> f32 {
    let alpha2 = mx_square_f32(alpha);
    let lambdaL = sqrt(alpha2 + (1.0 - alpha2) * mx_square_f32(NdotL));
    let lambdaV = sqrt(alpha2 + (1.0 - alpha2) * mx_square_f32(NdotV));
    return (2.0 * NdotL * NdotV) / (lambdaL * NdotV + lambdaV * NdotL);
}

// Rational quadratic fit to Monte Carlo data for GGX directional albedo.
fn mx_ggx_dir_albedo_analytic(NdotV: f32, alpha: f32, F0: vec3f, F90: vec3f) -> vec3f {
    let x = NdotV;
    let y = alpha;
    let x2 = mx_square_f32(x);
    let y2 = mx_square_f32(y);

    // Rational polynomial fit — accumulate terms incrementally to avoid
    // deeply nested parenthesized expressions that exceed WGSL parser limits.
    var r = vec4f(0.1003, 0.9345, 1.0, 1.0);
    r = r + vec4f(-0.6303, -2.323, -1.765, 0.2281) * x;
    r = r + vec4f(9.748, 2.229, 8.263, 15.94) * y;
    r = r + vec4f(-2.038, -3.748, 11.53, -55.83) * (x * y);
    r = r + vec4f(29.34, 1.424, 28.96, 13.08) * x2;
    r = r + vec4f(-8.245, -0.7684, -7.507, 41.26) * y2;
    r = r + vec4f(-26.44, 1.436, -36.11, 54.9) * (x2 * y);
    r = r + vec4f(19.99, 0.2913, 15.86, 300.2) * (x * y2);
    r = r + vec4f(-5.448, 0.6286, 33.37, -285.1) * (x2 * y2);

    let AB = clamp(r.xy / r.zw, vec2f(0.0), vec2f(1.0));
    return F0 * AB.x + F90 * AB.y;
}

fn mx_ggx_dir_albedo(NdotV: f32, alpha: f32, F0: vec3f, F90: vec3f) -> vec3f {
    return mx_ggx_dir_albedo_analytic(NdotV, alpha, F0, F90);
}

// GGX energy compensation: accounts for energy lost to multiple scattering.
fn mx_ggx_energy_compensation(NdotV: f32, alpha: f32, Fss: vec3f) -> vec3f {
    let Ess = mx_ggx_dir_albedo(NdotV, alpha, vec3f(1.0), vec3f(1.0));
    return vec3f(1.0) + Fss * (vec3f(1.0) - Ess) / max(Ess, vec3f(M_FLOAT_EPS));
}

// https://ggx-research.github.io/publication/2023/06/09/publication-ggx.html
fn mx_ggx_importance_sample_VNDF(Xi: vec2f, V: vec3f, alpha: vec2f) -> vec3f {
    let V_h = normalize(vec3f(V.xy * alpha, V.z));

    let phi = 2.0 * M_PI * Xi.x;
    let z = (1.0 - Xi.y) * (1.0 + V_h.z) - V_h.z;
    let sinTheta = sqrt(clamp(1.0 - z * z, 0.0, 1.0));
    let x = sinTheta * cos(phi);
    let y = sinTheta * sin(phi);
    let c = vec3f(x, y, z);

    let H = c + V_h;
    return normalize(vec3f(H.xy * alpha, max(H.z, 0.0)));
}

// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
// Equation 34
fn mx_ggx_smith_G1(cosTheta: f32, alpha: f32) -> f32 {
    let cosTheta2 = mx_square_f32(cosTheta);
    let tanTheta2 = (1.0 - cosTheta2) / cosTheta2;
    return 2.0 / (1.0 + sqrt(1.0 + mx_square_f32(alpha) * tanTheta2));
}

// Compute the refraction of a ray through a solid sphere.
fn mx_refraction_solid_sphere(R_in: vec3f, N: vec3f, ior: f32) -> vec3f {
    let R = refract(R_in, N, 1.0 / ior);
    let N1 = normalize(R * dot(R, N) - N * 0.5);
    return refract(R, N1, ior);
}

fn mx_latlong_projection(dir: vec3f) -> vec2f {
    let latitude = -asin(dir.y) * M_PI_INV + 0.5;
    let longitude = atan2(dir.x, -dir.z) * M_PI_INV * 0.5 + 0.5;
    return vec2f(longitude, latitude);
}

// Sample a lat-long environment map with the given direction, transform, and mip level.
// The texture and sampler are provided by the generator ($texSamplerSignature).
fn mx_latlong_map_lookup(dir: vec3f, transform: mat4x4f, lod: f32, envTex: texture_2d<f32>, envSampler: sampler) -> vec3f {
    let envDir = normalize((transform * vec4f(dir, 0.0)).xyz);
    let uv = mx_latlong_projection(envDir);
    return textureSampleLevel(envTex, envSampler, uv, lod).rgb;
}

// Return the mip level with appropriate coverage for a filtered importance sample.
// https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html
// Section 20.4 Equation 13
fn mx_latlong_compute_lod(dir: vec3f, pdf: f32, maxMipLevel: f32, envSamples: i32) -> f32 {
    let MIP_LEVEL_OFFSET = 1.5;
    let effectiveMaxMipLevel = maxMipLevel - MIP_LEVEL_OFFSET;
    let distortion = sqrt(1.0 - mx_square_f32(dir.y));
    return max(effectiveMaxMipLevel - 0.5 * log2(f32(envSamples) * pdf * distortion), 0.0);
}
