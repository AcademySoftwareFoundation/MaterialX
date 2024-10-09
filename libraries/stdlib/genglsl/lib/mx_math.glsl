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

// GLSL uses atan(x,y) and MSL uses atan2(x,y)
// follow the GLSL convention but prefix with mx_ to ensure consistent function
// names between languages.

// atan()
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

// the trigonometric functions below are one-to-one with their original counterparts
// in GLSL and MSL - just here to ensure we mx_ prefix families of functions

// cos()
float mx_cos(float x)
{
    return cos(x);
}
vec2 mx_cos(vec2 x)
{
    return cos(x);
}
vec3 mx_cos(vec3 x)
{
    return cos(x);
}
vec4 mx_cos(vec4 x)
{
    return cos(x);
}

// sin()
float mx_sin(float x)
{
    return sin(x);
}
vec2 mx_sin(vec2 x)
{
    return sin(x);
}
vec3 mx_sin(vec3 x)
{
    return sin(x);
}
vec4 mx_sin(vec4 x)
{
    return sin(x);
}

// tan()
float mx_tan(float x)
{
    return tan(x);
}
vec2 mx_tan(vec2 x)
{
    return tan(x);
}
vec3 mx_tan(vec3 x)
{
    return tan(x);
}
vec4 mx_tan(vec4 x)
{
    return tan(x);
}

// acos()
float mx_acos(float x)
{
    return acos(x);
}
vec2 mx_acos(vec2 x)
{
    return acos(x);
}
vec3 mx_acos(vec3 x)
{
    return acos(x);
}
vec4 mx_acos(vec4 x)
{
    return acos(x);
}

// asin()
float mx_asin(float x)
{
    return asin(x);
}
vec2 mx_asin(vec2 x)
{
    return asin(x);
}
vec3 mx_asin(vec3 x)
{
    return asin(x);
}
vec4 mx_asin(vec4 x)
{
    return asin(x);
}