#include "pbrlib/genglsl/lib/mx_microfacet_diffuse.glsl"

void mx_anisotropic_vdf_reflection(vec3 absorption, vec3 scattering, float anisotropy, inout BSDF bsdf)
{
    bsdf.throughput *= absorption;
}

void mx_anisotropic_vdf_transmission(vec3 absorption, vec3 scattering, float anisotropy, inout BSDF bsdf)
{
    bsdf.result *= absorption;
    bsdf.throughput *= absorption;
}
