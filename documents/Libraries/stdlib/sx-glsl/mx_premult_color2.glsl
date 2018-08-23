void mx_premult_color2(vec2 in, out vec2 result)
{
    result = vec2(in.r * in.a, in.a);
}
