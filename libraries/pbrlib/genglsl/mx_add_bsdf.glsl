void mx_add_bsdf(BSDF in1, BSDF in2, out BSDF result)
{
    result.response = in1.response + in2.response;
    result.throughput = in1.throughput + in2.throughput;
}

void mx_add_bsdf_reflection(vec3 _unused1, vec3 _unused2, vec3 _unused3, float weight, BSDF in1, BSDF in2, out BSDF result)
{
    mx_add_bsdf(in1, in2, result);
}

void mx_add_bsdf_transmission(vec3 _unused1, BSDF in1, BSDF in2, out BSDF result)
{
    mx_add_bsdf(in1, in2, result);
}

void mx_add_bsdf_indirect(vec3 _unused1, BSDF in1, BSDF in2, out BSDF result)
{
    mx_add_bsdf(in1, in2, result);
}
