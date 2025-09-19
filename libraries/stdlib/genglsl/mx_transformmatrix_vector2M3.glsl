void mx_transformmatrix_vector2M3(vec2 val, mat3 transform, out vec2 result)
{
  vec3 res = mx_matrix_mul(transform, vec3(val, 1.0));
  result = res.xy;
}
