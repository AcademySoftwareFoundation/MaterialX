#define M_FLOAT_EPS 1e-8

#define mx_mod mod
#define mx_inverse inverse
#define mx_inversesqrt inversesqrt
#define mx_sin sin
#define mx_cos cos
#define mx_tan tan
#define mx_asin asin
#define mx_acos acos
#define mx_atan atan
#define mx_radians radians

float mx_square(float x)
{
    return x*x;
}

vec2 mx_square(vec2 x)
{
    return x*x;
}

vec3 mx_square(vec3 x)
{
    return x*x;
}

vec3 mx_srgb_encode(vec3 color)
{
    bvec3 isAbove = greaterThan(color, vec3(0.0031308));
    vec3 linSeg = color * 12.92;
    vec3 powSeg = 1.055 * pow(max(color, vec3(0.0)), vec3(1.0 / 2.4)) - 0.055;
    return mix(linSeg, powSeg, isAbove);
}

mat3 mx_axis_rotation_matrix(vec3 a, float r)
{
    float s = sin(r);
    float c = cos(r);
    float omc = 1.0 - c;
    return mat3(
        a.x*a.x*omc + c,     a.x*a.y*omc - a.x*s, a.x*a.z*omc + a.y*s,
        a.y*a.x*omc + a.z*s, a.y*a.y*omc + c,     a.y*a.z*omc - a.x*s,
        a.z*a.x*omc - a.y*s, a.z*a.y*omc + a.x*s, a.z*a.z*omc + c
    );
}
