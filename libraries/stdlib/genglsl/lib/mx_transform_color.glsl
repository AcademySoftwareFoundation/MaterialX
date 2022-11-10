#define M_AP1_TO_REC709 mat3(1.705079555511475, -0.1297005265951157, -0.02416634373366833, -0.6242334842681885, 1.138468623161316, -0.1246141716837883, -0.0808461606502533, -0.008768022060394287, 1.148780584335327)

vec3 mx_srgb_texture_to_lin_rec709(vec3 color)
{
    bvec3 isAbove = greaterThan(color, vec3(0.04045));
    vec3 linSeg = color / 12.92;
    vec3 powSeg = pow(max(color + vec3(0.055), vec3(0.0)) / 1.055, vec3(2.4));
    return mix(linSeg, powSeg, isAbove);
}

vec3 mx_lin_adobergb_to_lin_rec709(vec3 color)
{
    mat3 adobergb_to_srgb = mat3( 1.39835574e+00, -2.50233861e-16,  2.77555756e-17,
                                 -3.98355744e-01,  1.00000000e+00, -4.29289893e-02,
                                  0.00000000e+00,  0.00000000e+00,  1.04292899e+00);
    return adobergb_to_srgb * color;
}

vec3 mx_adobergb_to_lin_rec709(vec3 color)
{
    vec3 lin_adobergb_color = pow(max(vec3(0.), color), vec3(563 / 256));
    return mx_lin_adobergb_to_lin_rec709(lin_adobergb_color);
}
