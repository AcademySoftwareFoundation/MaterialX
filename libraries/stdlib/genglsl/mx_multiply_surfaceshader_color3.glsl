void mx_multiply_surfaceshader_color3(surfaceshader shader1, vec3 value, out surfaceshader returnshader)
{
    returnshader.color = shader1.color * value;
    returnshader.transparency = shader1.transparency;
}
