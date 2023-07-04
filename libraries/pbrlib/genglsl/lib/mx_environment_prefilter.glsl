#include "mx_microfacet_specular.glsl"

float mx_latlong_compute_lod(float alpha)
{
    // Select a mip level based on input alpha.
    float lodBias = alpha < 0.25 ? sqrt(alpha) : 0.5*alpha + 0.375;
    return lodBias * float($envRadianceMips);
}

vec3 mx_environment_radiance(vec3 N, vec3 V, vec3 X, vec2 alpha, int distribution, FresnelData fd)
{
    N = mx_forward_facing_normal(N, V);
    vec3 L = fd.refraction ? mx_refraction_solid_sphere(-V, N, fd.ior.x) : -reflect(V, N);

    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    float avgAlpha = mx_average_alpha(alpha);
    vec3 F = mx_compute_fresnel(NdotV, fd);
    float G = mx_ggx_smith_G2(NdotV, NdotV, avgAlpha);
    // Convert n and k to f0. f90 will be equal to 1.
    // Source: Section 2.2: https://jcgt.org/published/0003/04/03/paper.pdf
    vec3 k2 = fd.extinction * fd.extinction;
    vec3 F0 = ((fd.ior - 1)*(fd.ior - 1) + k2) / ((fd.ior + 1)*(fd.ior + 1) + k2);
    vec3 FGD = mx_ggx_dir_albedo(NdotV, avgAlpha, F0, 1.0);
    if (fd.refraction) {
        FGD = 1.0 - FGD;
    }


    vec3 Li = mx_latlong_map_lookup(L, $envMatrix, mx_latlong_compute_lod(avgAlpha), $envRadiance);
    return Li * FGD;
}

vec3 mx_environment_irradiance(vec3 N)
{
    return mx_latlong_map_lookup(N, $envMatrix, 0.0, $envIrradiance);
}
