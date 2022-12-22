#include "lib/mx_transform_color.glsl"

void mx_lin_adobergb_to_lin_rec709_color3(vec3 _in, out vec3 result)
{
    result = ADOBERGB_TO_REC709 * _in;
}
