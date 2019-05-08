#include "stdlib/genglsl/mx_smoothstep_float.glsl"

void mx_smoothstep_vec2FA(vec2 val, float low, float high, out vec2 result)
{
    mx_smoothstep_float(val.x, low, high, result.x);
    mx_smoothstep_float(val.y, low, high, result.y);
}
