#include "lib/mx_closure_type.wgsl"
#include "lib/mx_microfacet_specular.wgsl"

fn mx_generalized_schlick_bsdf(
    closureData: ClosureData,
    weight: f32,
    color0: vec3f,
    color82: vec3f,
    color90: vec3f,
    exponent: f32,
    roughness: vec2f,
    retroreflective: bool,
    thinfilm_thickness: f32,
    thinfilm_ior: f32,
    N: vec3f,
    X: vec3f,
    distribution: i32,
    scatter_mode: i32,
    bsdf: ptr<function, BSDF>
) {
    if (weight < M_FLOAT_EPS) {
        return;
    }

    if (closureData.closureType != CLOSURE_TYPE_TRANSMISSION && scatter_mode == 1) {
        return;
    }

    let V = closureData.V;
    let L = closureData.L;

    var N_facing = mx_forward_facing_normal(N, V);
    let NdotV = mx_saturate(dot(N_facing, V));

    let safeColor0 = max(color0, vec3f(0.0));
    let safeColor82 = max(color82, vec3f(0.0));
    let safeColor90 = max(color90, vec3f(0.0));
    let fd = mx_init_fresnel_schlick(safeColor0, safeColor82, safeColor90, exponent, thinfilm_thickness, thinfilm_ior);

    let safeAlpha = clamp(roughness, vec2f(M_FLOAT_EPS), vec2f(1.0));
    let avgAlpha = mx_average_alpha(safeAlpha);

    if (closureData.closureType == CLOSURE_TYPE_REFLECTION) {
        var X_ortho = normalize(X - dot(X, N_facing) * N_facing);
        let Y = cross(N_facing, X_ortho);
        let H = normalize(L + V);

        let NdotL = mx_saturate(dot(N_facing, L));
        let VdotH = mx_saturate(dot(V, H));

        let Ht = vec3f(dot(H, X_ortho), dot(H, Y), dot(H, N_facing));

        let F = mx_compute_fresnel(VdotH, fd);
        let D = mx_ggx_NDF(Ht, safeAlpha);
        let G = mx_ggx_smith_G2(NdotL, NdotV, avgAlpha);

        let comp = mx_ggx_energy_compensation(NdotV, avgAlpha, F);
        let dirAlbedo = mx_ggx_dir_albedo(NdotV, avgAlpha, safeColor0, safeColor90) * comp;
        let avgDirAlbedo = dot(dirAlbedo, vec3f(1.0 / 3.0));
        (*bsdf).throughput = vec3f(1.0 - avgDirAlbedo * weight);

        (*bsdf).response = D * F * G * comp * closureData.occlusion * weight / (4.0 * NdotV);
    } else if (closureData.closureType == CLOSURE_TYPE_TRANSMISSION) {
        let F = mx_compute_fresnel(NdotV, fd);

        let comp = mx_ggx_energy_compensation(NdotV, avgAlpha, F);
        let dirAlbedo = mx_ggx_dir_albedo(NdotV, avgAlpha, safeColor0, safeColor90) * comp;
        let avgDirAlbedo = dot(dirAlbedo, vec3f(1.0 / 3.0));
        (*bsdf).throughput = vec3f(1.0 - avgDirAlbedo * weight);

        if (scatter_mode != 0) {
            (*bsdf).response = mx_surface_transmission(N_facing, V, X, safeAlpha, distribution, fd, vec3f(1.0)) * weight;
        }
    } else if (closureData.closureType == CLOSURE_TYPE_INDIRECT) {
        let F = mx_compute_fresnel(NdotV, fd);

        let comp = mx_ggx_energy_compensation(NdotV, avgAlpha, F);
        let dirAlbedo = mx_ggx_dir_albedo(NdotV, avgAlpha, safeColor0, safeColor90) * comp;
        let avgDirAlbedo = dot(dirAlbedo, vec3f(1.0 / 3.0));
        (*bsdf).throughput = vec3f(1.0 - avgDirAlbedo * weight);

        // FIS bakes the Fresnel term into Li, so weight only by the energy-compensation term
        // (matching mx_generalized_schlick_bsdf.glsl); dirAlbedo would double-count Fresnel.
        let Li = mx_environment_radiance(N_facing, V, X, safeAlpha, distribution, fd);
        (*bsdf).response = Li * comp * weight;
    }
}
