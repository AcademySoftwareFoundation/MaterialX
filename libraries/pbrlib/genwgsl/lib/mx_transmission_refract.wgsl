#include "mx_microfacet_specular.wgsl"

fn mx_surface_transmission(N: vec3f, V: vec3f, X: vec3f, alpha: vec2f, distribution: i32, fd: FresnelData, tint_in: vec3f) -> vec3f
{
    // Approximate the appearance of surface transmission as glossy environment map
    // refraction, ignoring any scene geometry that might be visible through the surface.
    // Setting fd.refraction makes mx_environment_radiance sample along the refracted ray
    // (mx_refraction_solid_sphere) with an inverted Fresnel weight, rather than reflecting.
    var fd_refract: FresnelData = fd;
    fd_refract.refraction = true;

    var tint: vec3f = tint_in;

    if (bool($refractionTwoSided)) {
        tint = mx_square_vec3(tint);
    }

    return mx_environment_radiance(N, V, X, alpha, distribution, fd_refract) * tint;
}
