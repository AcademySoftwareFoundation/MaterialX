// Microfacet utility functions (WGSL port of genglsl/lib/mx_microfacet.glsl)

const M_PI_INV: f32 = 1.0 / 3.1415926535897932;

fn mx_pow5(x: f32) -> f32 {
    let x2 = x * x;
    return x2 * x2 * x;
}

fn mx_pow6(x: f32) -> f32 {
    let x2 = x * x;
    return x2 * x2 * x2;
}

// Fresnel-Schlick variants — suffixed by return type and parameter set.

// F0 only (scalar)
fn mx_fresnel_schlick_f32(cosTheta: f32, F0: f32) -> f32 {
    let x = clamp(1.0 - cosTheta, 0.0, 1.0);
    return F0 + (1.0 - F0) * mx_pow5(x);
}

// F0 only (vec3)
fn mx_fresnel_schlick_vec3(cosTheta: f32, F0: vec3f) -> vec3f {
    let x = clamp(1.0 - cosTheta, 0.0, 1.0);
    return F0 + (vec3f(1.0) - F0) * mx_pow5(x);
}

// F0 + F90 (scalar)
fn mx_fresnel_schlick_f32_f90(cosTheta: f32, F0: f32, F90: f32) -> f32 {
    let x = clamp(1.0 - cosTheta, 0.0, 1.0);
    return mix(F0, F90, mx_pow5(x));
}

// F0 + F90 (vec3)
fn mx_fresnel_schlick_vec3_f90(cosTheta: f32, F0: vec3f, F90: vec3f) -> vec3f {
    let x = clamp(1.0 - cosTheta, 0.0, 1.0);
    return mix(F0, F90, vec3f(mx_pow5(x)));
}

// F0 + F90 + exponent (scalar)
fn mx_fresnel_schlick_f32_exp(cosTheta: f32, F0: f32, F90: f32, exponent: f32) -> f32 {
    let x = clamp(1.0 - cosTheta, 0.0, 1.0);
    return mix(F0, F90, pow(x, exponent));
}

// F0 + F90 + exponent (vec3)
fn mx_fresnel_schlick_vec3_exp(cosTheta: f32, F0: vec3f, F90: vec3f, exponent: f32) -> vec3f {
    let x = clamp(1.0 - cosTheta, 0.0, 1.0);
    return mix(F0, F90, vec3f(pow(x, exponent)));
}

fn mx_forward_facing_normal(N: vec3f, V: vec3f) -> vec3f {
    if (dot(N, V) < 0.0) {
        return -N;
    }
    return N;
}

// Spherical Fibonacci sampling utilities.
fn mx_golden_ratio_sequence(i: i32) -> f32 {
    let GOLDEN_RATIO = 1.618034;
    return fract((f32(i) + 1.0) * GOLDEN_RATIO);
}

fn mx_spherical_fibonacci(i: i32, numSamples: i32) -> vec2f {
    return vec2f((f32(i) + 0.5) / f32(numSamples), mx_golden_ratio_sequence(i));
}

fn mx_uniform_sample_hemisphere(Xi: vec2f) -> vec3f {
    let phi = 2.0 * 3.1415926535897932 * Xi.x;
    let cosTheta = 1.0 - Xi.y;
    let sinTheta = sqrt(1.0 - mx_square_f32(cosTheta));
    return vec3f(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

fn mx_cosine_sample_hemisphere(Xi: vec2f) -> vec3f {
    let phi = 2.0 * 3.1415926535897932 * Xi.x;
    let cosTheta = sqrt(Xi.y);
    let sinTheta = sqrt(1.0 - Xi.y);
    return vec3f(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

// Constructs an orthonormal basis (T, B, N) from a unit normal N.
// Uses the Duff et al. (JCGT 2017) / Frisvad construction with the sign branch
// at N.z = -1 instead of N.z = 0.  The original Naga-transpiled version branched
// at N.z < 0, which placed a tangent-frame discontinuity at the equator of any
// Z-up sphere -- causing visible sawtooth artifacts on glossy/metallic materials.
// Moving the branch to N.z < -0.9999999 pushes the singularity to a tiny region
// near the south pole where it is virtually invisible.
fn mx_orthonormal_basis(N: vec3f) -> mat3x3f {
    let s = select(1.0, -1.0, N.z < -0.9999999);
    let a = -1.0 / (s + N.z);
    let b = N.x * N.y * a;
    let X = vec3f(1.0 + s * N.x * N.x * a, s * b, -s * N.x);
    let Y = vec3f(b, s + N.y * N.y * a, -N.y);
    return mat3x3f(X, Y, N);
}
