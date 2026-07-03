#include "lib/mx_closure_type.wgsl"
#include "lib/mx_microfacet_specular.wgsl"

fn mx_conductor_bsdf(closureData: ClosureData, weight: f32, ior_n: vec3f, ior_k: vec3f, roughness: vec2f, retroreflective: bool, thinfilm_thickness: f32, thinfilm_ior: f32, N: vec3f, X: vec3f, distribution: i32, bsdf: ptr<function, BSDF>)
{
    (*bsdf).throughput = vec3f(0.0);

    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    var V: vec3f = closureData.V;
    var L: vec3f = closureData.L;
    var N_: vec3f = mx_forward_facing_normal(N, V);  // mutable copy (WGSL params are immutable)
    var X_: vec3f = X;

    var NdotV: f32 = clamp(dot(N_, V), M_FLOAT_EPS, 1.0);

    var fd: FresnelData = mx_init_fresnel_conductor(ior_n, ior_k, thinfilm_thickness, thinfilm_ior);

    var safeAlpha: vec2f = clamp(roughness, vec2f(M_FLOAT_EPS), vec2f(1.0));
    var avgAlpha: f32 = mx_average_alpha(safeAlpha);

    if (closureData.closureType == CLOSURE_TYPE_REFLECTION)
    {
        X_ = normalize(X_ - dot(X_, N_) * N_);
        var Y: vec3f = cross(N_, X_);
        var H: vec3f = normalize(L + V);

        var NdotL: f32 = clamp(dot(N_, L), M_FLOAT_EPS, 1.0);
        var VdotH: f32 = clamp(dot(V, H), M_FLOAT_EPS, 1.0);

        var Ht: vec3f = vec3f(dot(H, X_), dot(H, Y), dot(H, N_));

        var F: vec3f = mx_compute_fresnel(VdotH, fd);
        var D: f32 = mx_ggx_NDF(Ht, safeAlpha);
        var G: f32 = mx_ggx_smith_G2(NdotL, NdotV, avgAlpha);

        var comp: vec3f = mx_ggx_energy_compensation(NdotV, avgAlpha, F);

        // Note: NdotL is cancelled out
        (*bsdf).response = D * F * G * comp * closureData.occlusion * weight / (4.0 * NdotV);
    }
    else if (closureData.closureType == CLOSURE_TYPE_INDIRECT)
    {
        var F: vec3f = mx_compute_fresnel(NdotV, fd);
        var comp: vec3f = mx_ggx_energy_compensation(NdotV, avgAlpha, F);
        // FIS bakes the Fresnel term into Li, so weight only by the energy-compensation term
        // (matching mx_conductor_bsdf.glsl); multiplying by dirAlbedo would double-count Fresnel.
        var Li: vec3f = mx_environment_radiance(N_, V, X_, safeAlpha, distribution, fd);
        (*bsdf).response = Li * comp * weight;
    }
}
