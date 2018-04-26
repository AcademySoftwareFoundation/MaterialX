float sx_square(float x)
{
    return x*x;
}

vec2 sx_square(vec2 x)
{
    return x*x;
}

float sx_max_component(vec2 v)
{
    return max(v.x, v.y);
}

float sx_max_component(vec3 v)
{
    return max(max(v.x, v.y), v.z);
}

float sx_max_component(vec4 v)
{
    return max(max(max(v.x, v.y), v.z), v.w);
}

vec3 sx_front_facing(vec3 normal)
{
    return mix(-normal, normal, float(gl_FrontFacing));
}
