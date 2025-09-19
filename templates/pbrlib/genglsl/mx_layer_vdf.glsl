#include "lib/mx_closure_type.glsl"

void mx_layer_vdf(ClosureData closureData, BSDF top, BSDF base, out BSDF result)
{
    result.response = top.response + base.response;
    result.throughput = top.throughput + base.throughput;
}
