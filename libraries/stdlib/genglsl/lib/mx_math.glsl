// This file, and the corresponding mx_math.msl provide functions with a common interface, to allow
// code to be shared between GLSL and MSL.
// Guideline : prefix any functions with `mx_` to ensure no conflict with code that other shader
// generators (like HdStorm) might create.

#define M_FLOAT_EPS 1e-8

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

float mx_inversesqrt(float x)
{
    return inversesqrt(x);
}

float mx_radians(float degree)
{
    return radians(degree);
}

float mx_atan(float y_over_x)
{
    return atan(y_over_x);
}

float mx_atan(float y, float x)
{
    return atan(y, x);
}
vec2 mx_atan(vec2 y, vec2 x)
{
    return atan(y, x);
}
vec3 mx_atan(vec3 y, vec3 x)
{
    return atan(y, x);
}
vec4 mx_atan(vec4 y, vec4 x)
{
    return atan(y, x);
}
