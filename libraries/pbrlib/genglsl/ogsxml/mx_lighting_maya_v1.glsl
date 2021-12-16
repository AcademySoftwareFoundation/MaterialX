// This is the same algorithm as found in libraries\pbrlib\genglsl\lib\mx_environment_prefilter.glsl
// but adjusted for Maya. At this time we will compute a roughness based on the radiance and
// irradiance samples, so materials with small amount of roughness will look wrong.
//
// A more precise roughness computation can be done using Maya samplers, but this requires
// knowing that the Maya sampling functions are there, otherwise compilation will fail unless
// there is an IBL active in the Maya lighting.

#include "pbrlib/genglsl/lib/mx_microfacet_specular.glsl"

vec3 mx_environment_irradiance(vec3 N)
{
    return g_diffuseI;
}

vec3 mx_environment_radiance(vec3 N, vec3 V, vec3 X, vec2 roughness, int distribution, FresnelData fd)
{
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);
    float avgRoughness = mx_average_roughness(roughness);
    vec3 F = mx_compute_fresnel(NdotV, fd);
    float G = mx_ggx_smith_G2(NdotV, NdotV, avgRoughness);
    vec3 comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);
    vec3 Li = mix(g_specularI, g_diffuseI, avgRoughness);
    return Li * F * G * comp;
};
