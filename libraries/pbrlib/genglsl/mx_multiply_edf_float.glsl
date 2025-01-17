void mx_multiply_edf_float(vec3 _unused1, vec3 _unused2, EDF in1, float in2, out EDF result)
{
    result = in1 * in2;
}

//void mx_multiply_edf_float_reflection(vec3 _unused1, vec3 _unused2, vec3 _unused3, float weight, EDF in1, color3 in2, out EDF result)
//{
//    mx_multiply_edf_float(in1, in2, result);
//}
//
//void mx_multiply_edf_float_transmission(vec3 _unused1, EDF in1, float in2, out EDF result)
//{
//    mx_multiply_edf_float(in1, in2, result);
//}
//
//void mx_multiply_edf_float_indirect(vec3 _unused1, EDF in1, float in2, out EDF result)
//{
//    mx_multiply_edf_float(in1, in2, result);
//}
