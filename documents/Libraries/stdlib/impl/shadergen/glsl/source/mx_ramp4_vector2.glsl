void mx_ramp4_vector2(vec2 valuetl, vec2 valuetr, vec2 valuebl, vec2 valuebr, vec2 texcoord, out vec2 result)
{
    float ss = clamp(texcoord.x, 0, 1);
    float tt = clamp(texcoord.y, 0, 1);
    result = mix(mix(valuetl, valuetr, ss),
                 mix(valuebl, valuebr, ss),
                 tt);
}
