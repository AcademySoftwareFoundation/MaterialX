void sx_layeredsurface(surfaceshader top, surfaceshader base, float weight, out surfaceshader result)
{
    weight = saturate(weight);
    result.color = top.color * weight + base.color * (1.0 - weight);
    result.transparency = top.transparency * weight + base.transparency * (1.0 - weight);
}
