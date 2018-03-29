void mx_ramp4_vector4(vec4 valuetl, vec4 valuetr, vec4 valuebl, vec4 valuebr, vec2 texcoord, out vec4 result)
{
    float ss = clamp(texcoord.x, 0, 1);
    float tt = clamp(texcoord.y, 0, 1);
    result = mix(mix(valuetl, valuetr, ss),
                 mix(valuebl, valuebr, ss),
                 tt);
}
