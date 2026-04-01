#include "lib/mx_closure_type.glsl"

void mx_anisotropic_vdf(ClosureData closureData, vec3 absorption, vec3 scattering, float anisotropy, inout VDF vdf)
{
    // TODO: Add support for scattering and anisotropy.
    if (closureData.closureType == CLOSURE_TYPE_TRANSMISSION)
    {
        vdf.response = vec3(0.0);
        vdf.throughput = exp(-absorption);
    }
}
