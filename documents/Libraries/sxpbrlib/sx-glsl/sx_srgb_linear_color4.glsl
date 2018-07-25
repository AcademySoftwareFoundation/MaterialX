void sx_srgb_linear_color4(vec4 _in, out vec4 result)
{
    result = vec4(pow(_in.xyz, vec3(2.2)), _in.w);
}
