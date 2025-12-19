void mx_ramptb_vector4(vec4 valuet, vec4 valueb, vec2 texcoord, out vec4 result)
{
    result = mix (valueb, valuet, clamp(texcoord.y, 0.0, 1.0) );
}
