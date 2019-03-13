void mx_multiply_bsdf_float_reflection(vec3 L, vec3 V, BSDF in1, float in2, out BSDF result)
{
    result = in1 * clamp(in2, 0.0, 1.0);
}

void mx_multiply_bsdf_float_transmission(vec3 V, BSDF in1, float in2, out BSDF result)
{
    result = in1 * clamp(in2, 0.0, 1.0);
}

void mx_multiply_bsdf_float_indirect(vec3 V, vec3 in1, float in2, out vec3 result)
{
    result = in1 * clamp(in2, 0.0, 1.0);
}
