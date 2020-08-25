void mx_g22_ap1_to_lin_rec709_color4(vec4 _in, out vec4 result)
{
    vec4 outColor = vec4(_in.rgb, 0.);
    mat4 transform = mat4(1.705079555511475, -0.1297005265951157, -0.02416634373366833, 0., -0.6242334842681885, 1.138468623161316, -0.1246141716837883, 0., -0.0808461606502533, -0.008768022060394287, 1.148780584335327, 0., 0., 0., 0., 1.);
    vec3 gamma = vec3(2.2, 2.2, 2.2);
    result = vec4(pow(max(vec3(0., 0., 0.), (transform * outColor).rgb), gamma), _in.a);
}
