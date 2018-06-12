void sx_normalmap(vec3 value, vec3 N, vec3 T, out vec3 result)
{
    vec3 v = value * 2.0 - 1.0;
    vec3 B = normalize(cross(N, T));
    result = normalize(T * v.x + B * v.y + N * v.z);
}
