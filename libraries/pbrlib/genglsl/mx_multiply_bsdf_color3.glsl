#include "lib/mx_closure_type.glsl"

void mx_multiply_bsdf_color3(ClosureData closureData, BSDF in1, vec3 in2, out BSDF result)
{
    vec3 tint = clamp(in2, 0.0, 1.0);
    result.response = in1.response * tint;
    result.throughput = in1.throughput * tint;
}
