#include "mx_smoothstep_float.metal"

void mx_smoothstep_vec2FA(vec2 val, float low, float high, out vec2 result)
{
    float f;
    mx_smoothstep_float(val.x, low, high, f); result.x = f;
    mx_smoothstep_float(val.y, low, high, f); result.y = f;
}
