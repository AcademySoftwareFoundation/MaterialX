void mx_normalmap(vec3 value, int map_space, float normal_scale, vec3 N, vec3 T,  out vec3 result)
{
    // Decode the normal map.
    value = (value == vec3(0.0f)) ? vec3(0.0, 0.0, 1.0) : value * 2.0 - 1.0;

    // Transform from tangent space if needed.
    if (map_space == 0)
    {
        vec3 B = normalize(cross(N, T));
        value.xy *= normal_scale;
        value = T * value.x + B * value.y + N * value.z;
    }

    // Normalize the result.
    result = normalize(value);
}
