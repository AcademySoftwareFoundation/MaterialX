#include "mx_smoothstep_float.metal"

void mx_smoothstep_vec4FA(vec4 val, float low, float high, out vec4 result)
{
    float f;
    mx_smoothstep_float(val.x, low, high, f); result.x = f;
    mx_smoothstep_float(val.y, low, high, f); result.y = f;
    mx_smoothstep_float(val.z, low, high, f); result.z = f;
    mx_smoothstep_float(val.w, low, high, f); result.w = f;
}
