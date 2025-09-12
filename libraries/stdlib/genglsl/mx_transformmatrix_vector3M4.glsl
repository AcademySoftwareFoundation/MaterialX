void mx_transformmatrix_vector3M4(vec3 val, mat4 transform, out vec3 result)
{
  vec4 res = mx_matrix_mul(transform, vec4(val, 1.0));
  result = res.xyz;
}
