void mx_normalmap_vector2(vec3 value, vec2 normal_scale, vec3 N, vec3 T, vec3 B, out vec3 result)
{
    value = (dot(value, value) == 0.0) ? vec3(0.0, 0.0, 1.0) : value * 2.0 - 1.0;
    value = T * value.x * normal_scale.x +
            B * value.y * normal_scale.y +
            N * value.z;
    result = normalize(value);
}

void mx_normalmap_float(vec3 value, float normal_scale, vec3 N, vec3 T, vec3 B, out vec3 result)
{
    mx_normalmap_vector2(value, vec2(normal_scale), N, T, B, result);
}
