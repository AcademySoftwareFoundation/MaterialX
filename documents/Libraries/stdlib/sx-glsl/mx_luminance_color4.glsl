void mx_luminance_color4(vec4 in, vec3 lumacoeffs, out vec4 result)
{
    result = vec4(dot(_in.rgb, lumacoeffs), _in.a);
}
