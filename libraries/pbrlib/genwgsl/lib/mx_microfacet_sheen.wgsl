#include "mx_microfacet.wgsl"

// Imageworks sheen NDF
fn mx_imageworks_sheen_NDF(NdotH: f32, roughness: f32) -> f32 {
    let invRoughness = 1.0 / max(roughness, 0.005);
    let cos2 = NdotH * NdotH;
    let sin2 = 1.0 - cos2;
    return (2.0 + invRoughness) * pow(sin2, invRoughness * 0.5) / (2.0 * 3.1415926535897932);
}

// Missing MaterialX functions - Sheen BRDF and directional albedo
fn mx_imageworks_sheen_brdf(NdotL: f32, NdotV: f32, NdotH: f32, roughness: f32) -> f32 {
    let D = mx_imageworks_sheen_NDF(NdotH, roughness);
    let F = 1.0;
    let G = 1.0;
    return D * F * G / (4.0 * (NdotL + NdotV - NdotL * NdotV));
}

fn mx_imageworks_sheen_dir_albedo(NdotV: f32, roughness: f32) -> f32 {
    let x = NdotV;
    let y = roughness;
    let r = vec2(13.67300, 1.0) + vec2(-68.78018, 61.57746) * x + vec2(799.08825, 442.78211) * y + vec2(-905.00061, 2597.49308) * x * y + vec2(60.28956, 121.81241) * mx_square_f32(x) + vec2(1086.96473, 3045.55075) * mx_square_f32(y);
    return clamp(r.x / r.y, 0.0, 1.0);
}

fn mx_zeltner_sheen_dir_albedo(x: f32, y: f32) -> f32 {
    let s = y * (0.0206607 + 1.58491 * y) / (0.0379424 + y * (1.32227 + y));
    let m = y * (-0.193854 + y * (-1.14885 + y * (1.7932 - 0.95943 * y * y))) / (0.046391 + y);
    let o = y * (0.000654023 + (-0.0207818 + 0.119681 * y) * y) / (1.26264 + y * (-1.92021 + y));
    return exp(-0.5 * mx_square_f32((x - m) / s)) / (s * sqrt(2.0 * 3.1415926535897932)) + o;
}

fn mx_zeltner_sheen_ltc_aInv(x: f32, y: f32) -> f32 {
    return (2.58126 * x + 0.813703 * y) * y / (1.0 + 0.310327 * x * x + 2.60994 * x * y);
}

fn mx_zeltner_sheen_ltc_bInv(x: f32, y: f32) -> f32 {
    return sqrt(1.0 - x) * (y - 1.0) * y * y * y / (0.0000254053 + 1.71228 * x - 1.71506 * x * y + 1.34174 * y * y);
}

fn mx_transpose_mat3(m: mat3x3f) -> mat3x3f {
    return mat3x3f(
        vec3f(m[0].x, m[1].x, m[2].x),
        vec3f(m[0].y, m[1].y, m[2].y),
        vec3f(m[0].z, m[1].z, m[2].z)
    );
}

fn mx_orthonormal_basis_ltc(V: vec3f, N: vec3f, NdotV: f32) -> mat3x3f {
    var X = V - N * NdotV;
    let lenSqr = dot(X, X);
    if (lenSqr > 0.0) {
        X = X * (1.0 / sqrt(lenSqr));
        let Y = cross(N, X);
        return mat3x3f(vec3f(X.x, Y.x, N.x), vec3f(X.y, Y.y, N.y), vec3f(X.z, Y.z, N.z));
    }
    return mx_orthonormal_basis(N);
}

fn mx_zeltner_sheen_brdf(L: vec3f, V: vec3f, N: vec3f, NdotV: f32, roughness: f32) -> f32 {
    let basis = mx_orthonormal_basis_ltc(V, N, NdotV);
    let toLTC = mx_transpose_mat3(basis);
    let w = (toLTC * L);
    
    let aInv = mx_zeltner_sheen_ltc_aInv(NdotV, roughness);
    let bInv = mx_zeltner_sheen_ltc_bInv(NdotV, roughness);
    
    let wo = vec3f(aInv * w.x + bInv * w.z, aInv * w.y, w.z);
    let lenSqr = dot(wo, wo);
    
    return max(wo.z, 0.0) * (1.0 / 3.1415926535897932) * mx_square_f32(aInv / lenSqr);
}
