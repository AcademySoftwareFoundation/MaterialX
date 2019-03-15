void mx_saturate_color4(vec4 _in, float amount, vec3 lumacoeffs, out vec4 result)
{
    result = vec4(vec3(dot(_in.rgb, lumacoeffs)), _in.a);
    result = mix(result, _in, amount);
}
