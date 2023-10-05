void mx_creatematrix_vector3(vec3 in1, vec3 in2, vec3 in3, out mat3 result)
{
    result = mat3(in1.x, in1.y, in1.z,
                in2.x, in2.y, in2.z,
                in3.x, in3.y, in3.z);
}

void mx_creatematrix_vector4(vec4 in1, vec4 in2, vec4 in3, vec4 in4, out mat4 result)
{
    result = mat4(in1.x, in1.y, in1.z, in1.w,
                  in2.x, in2.y, in2.z, in2.w,
                  in3.x, in3.y, in3.z, in3.w,
                  in4.x, in4.y, in4.z, in4.w);
}