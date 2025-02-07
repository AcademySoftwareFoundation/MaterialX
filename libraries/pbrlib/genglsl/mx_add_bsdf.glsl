#include "lib/mx_closure_type.glsl"

void mx_add_bsdf(ClosureData closureData, BSDF in1, BSDF in2, out BSDF result)
{
    result.response = in1.response + in2.response;
    result.throughput = in1.throughput + in2.throughput;
}
