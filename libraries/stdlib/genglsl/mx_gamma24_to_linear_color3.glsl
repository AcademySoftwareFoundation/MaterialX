void mx_gamma24_to_linear_color3(vec3 _in, out vec3 result)
{
    vec3 gamma = vec3(2.4, 2.4, 2.4);
    result = pow( max( vec3(0., 0., 0.), _in ), gamma );
}
