void mx_unpremult_color3(vec3 _in, float alpha, out vec3 result)
{
    result = _in / alpha;
}
