#include "lib/mx_closure_type.glsl"

void mx_add_bsdf(ClosureData closureData, BSDF in1, BSDF in2, out BSDF result)
{
    // To provide a throughput computation for hardware shading languages,
    // we reinterpret BSDF addition as in1 + in2 = mix(in1, in2, 0.5) * 2.
    result.response = in1.response + in2.response;
    result.throughput = mix(in1.throughput, in2.throughput, 0.5);
}
