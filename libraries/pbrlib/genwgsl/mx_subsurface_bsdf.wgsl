#include "lib/mx_closure_type.wgsl"
#include "lib/mx_microfacet_diffuse.wgsl"

// NOTE: The GLSL version computes curvature via fwidth(N)/fwidth(P), but WGSL
// forbids fwidth inside non-uniform control flow (e.g. the light loop that
// calls this function).  We use a flat curvature approximation (0.0) for now.
// TODO: wire proper curvature when the renderer can pre-compute it per-vertex.

fn mx_subsurface_bsdf(closureData: ClosureData, weight: f32, color: vec3f, radius: vec3f, anisotropy: f32, N: vec3f, bsdf: ptr<function, BSDF>)
{
    (*bsdf).throughput = vec3f(0.0);

    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    var V: vec3f = closureData.V;
    var L: vec3f = closureData.L;
    var P: vec3f = closureData.P;
    var occlusion: f32 = closureData.occlusion;
    var N_: vec3f = mx_forward_facing_normal(N, V);

    if (closureData.closureType == CLOSURE_TYPE_REFLECTION)
    {
        // Flat curvature approximation — see note above.
        var curvature: f32 = 0.0;
        var sss: vec3f = mx_subsurface_scattering_approx(N_, L, P, color, radius, curvature);
        var NdotL: f32 = clamp(dot(N_, L), M_FLOAT_EPS, 1.0);
        var visibleOcclusion: f32 = 1.0 - NdotL * (1.0 - occlusion);
        (*bsdf).response = sss * visibleOcclusion * weight;
    }
    else if (closureData.closureType == CLOSURE_TYPE_INDIRECT)
    {
        // For now, we render indirect subsurface as simple indirect diffuse.
        var Li: vec3f = mx_environment_irradiance(N_);
        (*bsdf).response = Li * color * weight;
    }
}
