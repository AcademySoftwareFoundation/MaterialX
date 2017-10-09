// --------------------------------- Common Math Functions --------------------------------------

float square(float x)
{
    return x*x;
}

vec2 square(vec2 x)
{
    return x*x;
}

float maxv(vec2 v)
{
    return max(v.x, v.y);
}

float maxv(vec3 v)
{
    return max(max(v.x, v.y), v.z);
}

float maxv(vec4 v)
{
    return max(max(max(v.x, v.y), v.z), v.w);
}
