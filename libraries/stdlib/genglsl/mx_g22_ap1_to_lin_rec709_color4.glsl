void mx_g22_ap1_to_lin_rec709_color4(vec4 _in, out vec4 result)
{
    const mat3 AP1_TO_REC709 = mat3(1.705079555511475, -0.1297005265951157, -0.02416634373366833, -0.6242334842681885, 1.138468623161316, -0.1246141716837883, -0.0808461606502533, -0.008768022060394287, 1.148780584335327);
    vec3 linearColor = pow(max(vec3(0., 0., 0.), _in.rgb), vec3(2.2));
    result = vec4(AP1_TO_REC709 * linearColor, _in.a);
}
