void transformnormal3_matrix44(vec3 val, mat4 transform, out vec3 result)
{
    result = vec3(vec4(val, 0.0) * inverse(transform));
}
