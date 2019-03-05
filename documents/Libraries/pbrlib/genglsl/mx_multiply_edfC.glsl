void mx_multiply_edfC_reflection(vec3 L, vec3 V, EDF in1, vec3 in2, out EDF result)
{
    result = in1 * clamp(in2, 0.0, 1.0);
}

void mx_multiply_edfC_transmission(vec3 V, EDF in1, vec3 in2, out EDF result)
{
    result = in1 * clamp(in2, 0.0, 1.0);
}

void mx_multiply_edfC_indirect(vec3 V, vec3 in1, vec3 in2, out vec3 result)
{
    result = in1 * clamp(in2, 0.0, 1.0);
}
