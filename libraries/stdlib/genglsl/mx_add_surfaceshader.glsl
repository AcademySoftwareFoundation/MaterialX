void mx_add_surfaceshader(surfaceshader shader1, surfaceshader shader2, out surfaceshader returnshader)
{
    returnshader.color = shader1.color + shader2.color;
    returnshader.transparency = shader1.transparency + shader2.transparency;
}
