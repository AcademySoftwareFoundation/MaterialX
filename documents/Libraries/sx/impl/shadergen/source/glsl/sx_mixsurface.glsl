void sx_mixsurface(surfaceshader in1, surfaceshader in2, float weight, out surfaceshader result)
{
    result.color = in1.color * weight + in2.color * (1.0 - weight);
    result.transparency = in1.transparency * weight + in2.transparency * (1.0 - weight);
}
