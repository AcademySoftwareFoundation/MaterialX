#include "lib/mx_closure_type.glsl"

void mx_layer_vdf(ClosureData closureData, BSDF top, VDF base, out BSDF result)
{
    result.response = top.response * base.throughput;
    result.throughput = top.throughput * base.throughput;
}
