void mx_ramp4_float(float valuetl, float valuetr, float valuebl, float valuebr, vec2 texcoord, out float result)
{
    float ss = clamp(texcoord.x, 0, 1);
    float tt = clamp(texcoord.y, 0, 1);
    result = mix(mix(valuetl, valuetr, ss),
                 mix(valuebl, valuebr, ss),
                 tt);
}
