void mx_unpremult_color4(vec4 in, out vec4 result)
{
    result = vec4(in.rgb / in.a, in.a);
}
