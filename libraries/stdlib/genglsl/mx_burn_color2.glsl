#include "stdlib/genglsl/mx_burn_float.glsl"

void mx_burn_color2(vec2 fg, vec2 bg, float mixval, out vec2 result)
{
    mx_burn_float(fg.x, bg.x, mixval, result.x);
    mx_burn_float(fg.y, bg.y, mixval, result.y);
}
