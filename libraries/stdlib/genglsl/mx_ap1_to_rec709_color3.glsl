#include "lib/mx_transform_color.glsl"

void mx_ap1_to_rec709_color3(vec3 _in, out vec3 result)
{
    result = M_AP1_TO_REC709 * _in;
}
