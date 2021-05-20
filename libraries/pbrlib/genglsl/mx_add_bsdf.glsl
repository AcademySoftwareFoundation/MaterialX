void mx_add_bsdf_reflection(vec3 L, vec3 V, vec3 P, float occlusion, BSDF in1, BSDF in2, out BSDF result)
{
    result.eval = in1.eval + in2.eval;
    result.throughput = in1.throughput * in2.throughput;
}

void mx_add_bsdf_transmission(vec3 V, BSDF in1, BSDF in2, out BSDF result)
{
    result.eval = in1.eval + in2.eval;
    result.throughput = in1.throughput * in2.throughput;
}

void mx_add_bsdf_indirect(vec3 V, vec3 in1, vec3 in2, out vec3 result)
{
    result.eval = in1.eval + in2.eval;
    result.throughput = in1.throughput * in2.throughput;
}
