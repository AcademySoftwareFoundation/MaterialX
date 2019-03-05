void mx_add_bsdf_reflection(vec3 L, vec3 V, BSDF in1, BSDF in2, out BSDF result)
{
    result = in1 + in2;
}

void mx_add_bsdf_transmission(vec3 V, BSDF in1, BSDF in2, out BSDF result)
{
    result = in1 + in2;
}

void mx_add_bsdf_indirect(vec3 V, vec3 in1, vec3 in2, out vec3 result)
{
    result = in1 + in2;
}
