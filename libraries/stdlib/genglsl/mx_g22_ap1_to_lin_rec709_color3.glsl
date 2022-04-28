#include "lib/mx_transform_color.glsl"

void mx_g22_ap1_to_lin_rec709_color3(vec3 _in, out vec3 result)
{
    result = M_AP1_TO_REC709 * pow(max(vec3(0.0), _in), vec3(2.2));
}
