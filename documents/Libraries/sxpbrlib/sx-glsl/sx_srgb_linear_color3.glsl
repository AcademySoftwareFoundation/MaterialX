void sx_srgb_linear_color3(vec3 _in, out vec3 result)
{
    result = pow(_in, vec3(2.2));
}
