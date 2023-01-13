#include "lib/mx_transform_color.glsl"

void mx_lin_adobergb_to_lin_rec709_color4(vec4 _in, out vec4 result)
{
    result = vec4(ADOBERGB_TO_REC709 * _in.rgb, _in.a);
}
