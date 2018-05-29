float sx_square(float x)
{
    return x*x;
}

vec2 sx_square(vec2 x)
{
    return x*x;
}

vec3 sx_square(vec3 x)
{
    return x*x;
}

vec4 sx_square(vec4 x)
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

bool sx_is_tiny(float v)
{
    return v < M_FLOAT_EPS;
}

bool sx_is_tiny(vec3 v)
{
    return v.x < M_FLOAT_EPS && v.y < M_FLOAT_EPS && v.z < M_FLOAT_EPS;
}
