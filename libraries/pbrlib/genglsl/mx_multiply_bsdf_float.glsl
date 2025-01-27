void mx_multiply_bsdf_float(BSDF in1, float in2, out BSDF result)
{
    float weight = clamp(in2, 0.0, 1.0);
    result.response = in1.response * weight;
    result.throughput = in1.throughput * weight;
}

void mx_multiply_bsdf_float_reflection(vec3 _unused1, vec3 _unused2, vec3 _unused3, float weight, BSDF in1, float in2, out BSDF result)
{
    mx_multiply_bsdf_float(in1, in2, result);
}

void mx_multiply_bsdf_float_transmission(vec3 _unused1, BSDF in1, float in2, out BSDF result)
{
    mx_multiply_bsdf_float(in1, in2, result);
}

void mx_multiply_bsdf_float_indirect(vec3 _unused1, BSDF in1, float in2, out BSDF result)
{
    mx_multiply_bsdf_float(in1, in2, result);
}
