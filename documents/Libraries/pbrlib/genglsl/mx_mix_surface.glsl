void mx_mix_surface(surfaceshader in1, surfaceshader in2, float weight, out surfaceshader result)
{
    weight = clamp(weight, 0.0, 1.0);
    result.color = in1.color * (1.0 - weight) + in2.color * weight;
    result.transparency = in1.transparency * (1.0 - weight) + in2.transparency * weight;
}
