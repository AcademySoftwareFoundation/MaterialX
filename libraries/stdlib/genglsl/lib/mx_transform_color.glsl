const mat3 AP1_TO_REC709 = mat3( 1.705050992658, -0.130256417507, -0.024003356805,
                                -0.621792120657,  1.140804736575, -0.128968976065,
                                -0.083258872001, -0.010548319068,  1.15297233287);

const mat3 ADOBERGB_TO_REC709 = mat3( 1.39835574e+00, -2.50233861e-16,  2.77555756e-17,
                                     -3.98355744e-01,  1.00000000e+00, -4.29289893e-02,
                                      0.00000000e+00,  0.00000000e+00,  1.04292899e+00);

const float ADOBERGB_GAMMA = 563.0 / 256.0;

vec3 mx_srgb_texture_to_lin_rec709(vec3 color)
{
    bvec3 isAbove = greaterThan(color, vec3(0.04045));
    vec3 linSeg = color / 12.92;
    vec3 powSeg = pow(max(color + vec3(0.055), vec3(0.0)) / 1.055, vec3(2.4));
    return mix(linSeg, powSeg, vec3(isAbove));
}
