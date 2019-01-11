void mx_srgb_linear_color2(vec2 _in, out vec2 result)
{
    result = vec2(pow(_in.x, 2.2), _in.y);
}
