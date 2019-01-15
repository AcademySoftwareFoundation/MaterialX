void mx_gamma18_to_linear_color4(vec4 _in, out vec4 result)
{
    vec4 gamma = vec4(1.8, 1.8, 1.8, 1.);
    result = pow( max( vec4(0., 0., 0., 0.), _in ), gamma );
}
