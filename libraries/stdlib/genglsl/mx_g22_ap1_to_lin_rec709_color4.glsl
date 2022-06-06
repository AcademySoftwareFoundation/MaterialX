#include "lib/mx_transform_color.glsl"

void mx_g22_ap1_to_lin_rec709_color4(vec4 _in, out vec4 result)
{
    result = vec4(M_AP1_TO_REC709 * pow(max(vec3(0.0), _in.rgb), vec3(2.2)), _in.a);
}
