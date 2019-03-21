void mx_multiply_surfaceshader_float(surfaceshader shader1, float value, out surfaceshader returnshader)
{
    returnshader.color = shader1.color * vec3(value);
    returnshader.transparency = shader1.transparency * value;
}
