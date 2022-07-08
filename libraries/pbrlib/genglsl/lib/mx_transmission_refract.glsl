#include "mx_microfacet_specular.glsl"

vec3 mx_surface_transmission(vec3 N, vec3 V, vec3 X, vec2 alpha, int distribution, FresnelData fd)
{
    vec3 transmission;
    if ($refractionEnv)
    {
        // Approximate the appearance of surface transmission as glossy
        // environment map refraction, ignoring any scene geometry that might
        // be visible through the surface.
        fd.refraction = true;
        transmission = mx_environment_radiance(N, V, X, alpha, distribution, fd);
    }
    else
    {
        transmission = $refractionColor;
    }

    return transmission;
}
