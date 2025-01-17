void mx_multiply_bsdf_color3(BSDF in1, vec3 in2, out BSDF result)
{
    vec3 weight = clamp(in2, 0.0, 1.0);
    result.response = in1.response * weight;
    result.throughput = in1.throughput * weight;
}

void mx_multiply_bsdf_color3_reflection(vec3 _unused1, vec3 _unused2, vec3 _unused3, float _unused4, BSDF in1, vec3 in2, out BSDF result)
{
    mx_multiply_bsdf_color3(in1, in2, result);
}

void mx_multiply_bsdf_color3_transmission(vec3 _unused1, BSDF in1, vec3 in2, out BSDF result)
{
    mx_multiply_bsdf_color3(in1, in2, result);
}

void mx_multiply_bsdf_color3_indirect(vec3 _unused1, BSDF in1, vec3 in2, out BSDF result)
{
    mx_multiply_bsdf_color3(in1, in2, result);
}
