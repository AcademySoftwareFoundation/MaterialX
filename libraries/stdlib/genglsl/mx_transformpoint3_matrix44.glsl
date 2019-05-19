void transformpoint3_matrix44(vec3 val, mat4 transform, out vec3 result)
{
  vec4 res = transform * vec4(val, 1.0);
  result = res.xyz;
}
