#include "mx_microfacet.wgsl"

// Qualitative Oren-Nayar diffuse with simplified math:
// https://www1.cs.columbia.edu/CAVE/publications/pdfs/Oren_SIGGRAPH94.pdf
fn mx_oren_nayar_diffuse(NdotV: f32, NdotL: f32, LdotV: f32, roughness: f32) -> f32 {
    let s = LdotV - NdotL * NdotV;
    var stinv: f32;
    if (s > 0.0) {
        stinv = s / max(NdotL, NdotV);
    } else {
        stinv = 0.0;
    }

    let sigma2 = mx_square_f32(roughness);
    let A = 1.0 - 0.5 * (sigma2 / (sigma2 + 0.33));
    let B = 0.45 * sigma2 / (sigma2 + 0.09);

    return A + B * stinv;
}

// Missing MaterialX functions - Oren-Nayar compensated diffuse
const FUJII_CONSTANT_1: f32 = 0.5 - 2.0 / (3.0 * 3.1415926535897932);
const FUJII_CONSTANT_2: f32 = 2.0 / 3.0 - 28.0 / (15.0 * 3.1415926535897932);

fn mx_oren_nayar_fujii_diffuse_dir_albedo(cosTheta: f32, roughness: f32) -> f32 {
    let A = 1.0 / (1.0 + FUJII_CONSTANT_1 * roughness);
    let B = roughness * A;
    let Si = sqrt(max(0.0, 1.0 - mx_square_f32(cosTheta)));
    let cosThetaClamped = clamp(cosTheta, -1.0, 1.0);
    let G = Si * (acos(cosThetaClamped) - Si * cosTheta) + 2.0 * ((Si / max(cosTheta, 0.0001)) * (1.0 - Si * Si * Si) - Si) / 3.0;
    return A + (B * G * (1.0 / 3.1415926535897932));
}

fn mx_oren_nayar_fujii_diffuse_avg_albedo(roughness: f32) -> f32 {
    let A = 1.0 / (1.0 + FUJII_CONSTANT_1 * roughness);
    return A * (1.0 + FUJII_CONSTANT_2 * roughness);
}

fn mx_oren_nayar_compensated_diffuse(NdotV: f32, NdotL: f32, LdotV: f32, roughness: f32, color: vec3f) -> vec3f {
    let s = LdotV - NdotL * NdotV;
    var stinv: f32;
    if (s > 0.0) {
        stinv = s / max(NdotL, NdotV);
    } else {
        stinv = s;
    }
    
    let A = 1.0 / (1.0 + FUJII_CONSTANT_1 * roughness);
    let lobeSingleScatter = color * A * (1.0 + roughness * stinv);
    
    let dirAlbedoV = mx_oren_nayar_fujii_diffuse_dir_albedo(NdotV, roughness);
    let dirAlbedoL = mx_oren_nayar_fujii_diffuse_dir_albedo(NdotL, roughness);
    let avgAlbedo = mx_oren_nayar_fujii_diffuse_avg_albedo(roughness);
    let colorMultiScatter = mx_square_vec3(color) * avgAlbedo / (vec3(1.0) - color * max(0.0, 1.0 - avgAlbedo));
    let lobeMultiScatter = colorMultiScatter * max(0.00000001, 1.0 - dirAlbedoV) * max(0.00000001, 1.0 - dirAlbedoL) / max(0.00000001, 1.0 - avgAlbedo);
    
    return lobeSingleScatter + lobeMultiScatter;
}

fn mx_oren_nayar_compensated_diffuse_dir_albedo(cosTheta: f32, roughness: f32, color: vec3f) -> vec3f {
    let dirAlbedo = mx_oren_nayar_fujii_diffuse_dir_albedo(cosTheta, roughness);
    let avgAlbedo = mx_oren_nayar_fujii_diffuse_avg_albedo(roughness);
    let colorMultiScatter = mx_square_vec3(color) * avgAlbedo / (vec3(1.0) - color * max(0.0, 1.0 - avgAlbedo));
    return mix(colorMultiScatter, color, dirAlbedo);
}

fn mx_oren_nayar_diffuse_dir_albedo(NdotV: f32, roughness: f32) -> f32 {
    let x = NdotV;
    let y = roughness;
    let r = vec2(1.0, 1.0) + vec2(-0.4297, -0.6076) * y + vec2(-0.7632, -0.4993) * x * y + vec2(1.4385, 2.0315) * mx_square_f32(y);
    return clamp(r.x / r.y, 0.0, 1.0);
}

// Burley diffusion profile for subsurface scattering approximation.
// Ported from genglsl/lib/mx_microfacet_diffuse.glsl.
fn mx_burley_diffusion_profile(dist: f32, shape: vec3f) -> vec3f {
    let num1 = exp(-shape * dist);
    let num2 = exp(-shape * dist / 3.0);
    let denom = max(dist, M_FLOAT_EPS);
    return (num1 + num2) / denom;
}

// Integrate the Burley diffusion profile over a sphere of the given radius.
// Inspired by Eric Penner's presentation in http://advances.realtimerendering.com/s2011/
fn mx_integrate_burley_diffusion(N: vec3f, L: vec3f, radius: f32, mfp: vec3f) -> vec3f {
    let theta = acos(clamp(dot(N, L), -1.0, 1.0));

    // Estimate the Burley diffusion shape from mean free path.
    let shape = vec3f(1.0) / max(mfp, vec3f(0.1));

    // Integrate the profile over the sphere.
    var sumD = vec3f(0.0);
    var sumR = vec3f(0.0);
    let SAMPLE_COUNT: i32 = 32;
    let SAMPLE_WIDTH: f32 = (2.0 * M_PI) / f32(SAMPLE_COUNT);
    for (var i: i32 = 0; i < SAMPLE_COUNT; i++) {
        let x = -M_PI + (f32(i) + 0.5) * SAMPLE_WIDTH;
        let dist = radius * abs(2.0 * sin(x * 0.5));
        let R = mx_burley_diffusion_profile(dist, shape);
        sumD += R * max(cos(theta + x), 0.0);
        sumR += R;
    }

    return sumD / sumR;
}

fn mx_subsurface_scattering_approx(N: vec3f, L: vec3f, P: vec3f, albedo: vec3f, mfp: vec3f, curvature: f32) -> vec3f {
    let radius = 1.0 / max(curvature, 0.01);
    return albedo * mx_integrate_burley_diffusion(N, L, radius, mfp) / vec3f(M_PI);
}

// Disney Burley diffuse BRDF.
// https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf
// Section 5.3
fn mx_burley_diffuse(NdotV: f32, NdotL: f32, LdotH: f32, roughness: f32) -> f32 {
    let F90 = 0.5 + 2.0 * roughness * mx_square_f32(LdotH);
    let refL = mx_fresnel_schlick_f32_f90(NdotL, 1.0, F90);
    let refV = mx_fresnel_schlick_f32_f90(NdotV, 1.0, F90);
    return refL * refV;
}

// Directional albedo for Burley diffuse. Curve fit by Stephen Hill.
fn mx_burley_diffuse_dir_albedo(NdotV: f32, roughness: f32) -> f32 {
    let x = NdotV;
    let fit0 = 0.97619 - 0.488095 * mx_pow5(1.0 - x);
    let fit1 = 1.55754 + (-2.02221 + (2.56283 - 1.06244 * x) * x) * x;
    return mix(fit0, fit1, roughness);
}
