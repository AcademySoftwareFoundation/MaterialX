void mx_unpremult_color3(vec3 in, float alpha, out vec3 result)
{
    result = in / alpha;
}
