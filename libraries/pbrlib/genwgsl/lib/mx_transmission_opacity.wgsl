#include "mx_microfacet_specular.wgsl"

// Opacity-only surface transmission: no refraction, just tint.
fn mx_surface_transmission(N: vec3f, V: vec3f, X: vec3f, alpha: vec2f, distribution: i32, fd: FresnelData, tint: vec3f) -> vec3f {
    return tint;
}
