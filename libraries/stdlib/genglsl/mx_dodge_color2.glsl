#include "stdlib/genglsl/mx_dodge_float.glsl"

void mx_dodge_color2(vec2 fg, vec2 bg, float mixval, out vec2 result)
{
    mx_dodge_float(fg.x, bg.x, mixval, result.x);
    mx_dodge_float(fg.y, bg.y, mixval, result.y);
}
