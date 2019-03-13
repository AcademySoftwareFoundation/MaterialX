void mx_saturate_color3(vec3 _in, float amount, vec3 lumacoeffs, out vec3 result)
{
    result = vec3(dot(_in, lumacoeffs));
    result = mix(result, _in, amount);
}
