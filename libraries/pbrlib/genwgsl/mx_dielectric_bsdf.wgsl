#include "lib/mx_closure_type.wgsl"
#include "lib/mx_microfacet_specular.wgsl"

fn mx_dielectric_bsdf(closureData: ClosureData, weight: f32, tint: vec3f, ior: f32, roughness: vec2f, retroreflective: bool, thinfilm_thickness: f32, thinfilm_ior: f32, N_in: vec3f, X_in: vec3f, distribution: i32, scatter_mode: i32, bsdf: ptr<function, BSDF>) {
    if (weight < M_FLOAT_EPS) { return; }
    if (closureData.closureType != CLOSURE_TYPE_TRANSMISSION && scatter_mode == 1) { return; }

    let V = closureData.V;
    let L = closureData.L;
    let N = mx_forward_facing_normal(N_in, V);
    let NdotV = clamp(dot(N, V), 1e-8, 1.0);

    let fd = mx_init_fresnel_dielectric(ior, thinfilm_thickness, thinfilm_ior);
    let F0 = mx_ior_to_f0(ior);

    let safeAlpha = clamp(roughness, vec2f(1e-8), vec2f(1.0));
    let avgAlpha = mx_average_alpha(safeAlpha);
    let safeTint = max(tint, vec3f(0.0));

    if (closureData.closureType == CLOSURE_TYPE_REFLECTION) {
        var X = normalize(X_in - dot(X_in, N) * N);
        let Y = cross(N, X);
        let H = normalize(L + V);

        let NdotL = clamp(dot(N, L), 1e-8, 1.0);
        let VdotH = clamp(dot(V, H), 1e-8, 1.0);

        let Ht = vec3f(dot(H, X), dot(H, Y), dot(H, N));

        let F = mx_compute_fresnel(VdotH, fd);
        let D = mx_ggx_NDF(Ht, safeAlpha);
        let G = mx_ggx_smith_G2(NdotL, NdotV, avgAlpha);

        let comp = mx_ggx_energy_compensation(NdotV, avgAlpha, F);
        let dirAlbedo = mx_ggx_dir_albedo(NdotV, avgAlpha, vec3f(F0), vec3f(1.0)) * comp;
        (*bsdf).throughput = vec3f(1.0) - dirAlbedo * weight;

        (*bsdf).response = D * F * G * comp * safeTint * closureData.occlusion * weight / (4.0 * NdotV);
    } else if (closureData.closureType == CLOSURE_TYPE_TRANSMISSION) {
        let F = mx_compute_fresnel(NdotV, fd);

        let comp = mx_ggx_energy_compensation(NdotV, avgAlpha, F);
        let dirAlbedo = mx_ggx_dir_albedo(NdotV, avgAlpha, vec3f(F0), vec3f(1.0)) * comp;
        (*bsdf).throughput = vec3f(1.0) - dirAlbedo * weight;

        if (scatter_mode != 0) {
            (*bsdf).response = mx_surface_transmission(N, V, X_in, safeAlpha, distribution, fd, safeTint) * weight;
        }
    } else if (closureData.closureType == CLOSURE_TYPE_INDIRECT) {
        let F = mx_compute_fresnel(NdotV, fd);

        let comp = mx_ggx_energy_compensation(NdotV, avgAlpha, F);
        let dirAlbedo = mx_ggx_dir_albedo(NdotV, avgAlpha, vec3f(F0), vec3f(1.0)) * comp;
        (*bsdf).throughput = vec3f(1.0) - dirAlbedo * weight;

        // FIS bakes the Fresnel term into Li, so weight only by the energy-compensation term
        // (matching mx_dielectric_bsdf.glsl); dirAlbedo would double-count Fresnel.
        let Li = mx_environment_radiance(N, V, X_in, safeAlpha, distribution, fd);
        (*bsdf).response = Li * safeTint * comp * weight;
    }
}
