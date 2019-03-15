void transformpoint3_matrix44(vec3 val, mat4 transform, out vec3 result)
{
  vec4 _val = vec4(val.x, val.y, val.z, 1.0);
  vec4 res = transform * _val;
  result = res.xyz;
}
