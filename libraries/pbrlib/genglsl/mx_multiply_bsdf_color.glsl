void mx_multiply_bsdf_color_reflection(vec3 L, vec3 V, BSDF in1, vec3 in2, out BSDF result)
{
    result = in1 * clamp(in2, 0.0, 1.0);
}

void mx_multiply_bsdf_color_transmission(vec3 V, BSDF in1, vec3 in2, out BSDF result)
{
    result = in1 * clamp(in2, 0.0, 1.0);
}

void mx_multiply_bsdf_color_indirect(vec3 V, vec3 in1, vec3 in2, out vec3 result)
{
    result = in1 * clamp(in2, 0.0, 1.0);
}
