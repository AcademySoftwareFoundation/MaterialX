void mx_ramp4_vector3(vec3 valuetl, vec3 valuetr, vec3 valuebl, vec3 valuebr, vec2 texcoord, out vec3 result)
{
    float ss = clamp(texcoord.x, 0, 1);
    float tt = clamp(texcoord.y, 0, 1);
    result = mix(mix(valuetl, valuetr, ss),
                 mix(valuebl, valuebr, ss),
                 tt);
}
