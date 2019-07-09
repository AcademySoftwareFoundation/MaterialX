void mx_normalmap(vec3 value, int map_space, float normal_scale, vec3 N, vec3 T,  out vec3 result)
{
    // Tangent space
    if (map_space == 0)
    {
        vec3 v = value * 2.0 - 1.0;
        vec3 B = normalize(cross(N, T));
        result = normalize(T * v.x * normal_scale + B * v.y * normal_scale + N * v.z);
    }
    // Object space
    else
    {
        vec3 n = value * 2.0 - 1.0;
        result = normalize(n);
    }
}
