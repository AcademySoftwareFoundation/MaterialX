#include "lib/mx_transform_color.glsl"

void mx_adobergb_to_lin_rec709_color3(vec3 _in, out vec3 result)
{
    vec3 linearColor = pow(max(_in, vec3(0.0)), vec3(ADOBERGB_GAMMA));
    result = ADOBERGB_TO_REC709 * linearColor;
}
