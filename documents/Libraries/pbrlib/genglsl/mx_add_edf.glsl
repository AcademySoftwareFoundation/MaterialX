void mx_add_edf_reflection(vec3 L, vec3 V, EDF in1, EDF in2, out EDF result)
{
    result = in1 + in2;
}

void mx_add_edf_transmission(vec3 V, EDF in1, EDF in2, out EDF result)
{
    result = in1 + in2;
}

void mx_add_edf_indirect(vec3 V, vec3 in1, vec3 in2, out vec3 result)
{
    result = in1 + in2;
}
