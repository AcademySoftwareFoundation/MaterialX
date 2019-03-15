void mx_gamma18_to_linear_color3(vec3 _in, out vec3 result)
{
    vec3 gamma = vec3(1.8, 1.8, 1.8);
    result = pow( max( vec3(0., 0., 0.), _in ), gamma );
}
