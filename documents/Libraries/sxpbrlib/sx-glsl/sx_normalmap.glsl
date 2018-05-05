void sx_normalmap(vec3 value, vec3 normal, vec3 tangent, vec3 bitangent, out vec3 result)
{
    vec3 n = 2.0f * value - 1.0f;
    result = normalize(n.x * tangent + n.y * bitangent + n.z * normal);
}
