void mx_unpremult_color2(vec2 _in, out vec2 result)
{
    result = vec2(_in.r / _in.g, _in.g);
}
