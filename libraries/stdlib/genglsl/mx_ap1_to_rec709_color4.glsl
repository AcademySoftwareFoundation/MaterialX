#include "lib/mx_transform_color.glsl"

void mx_ap1_to_rec709_color4(vec4 _in, out vec4 result)
{
    result = vec4(M_AP1_TO_REC709 * _in.rgb, _in.a);
}
