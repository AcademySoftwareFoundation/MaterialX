void mx_mix_surfaceshader(surfaceshader shader1, surfaceshader shader2, float value, out surfaceshader returnshader)
{
    returnshader.color = mix(shader1.color, shader2.color, value);
    returnshader.transparency = mix(shader1.transparency, shader2.transparency, value);
}
