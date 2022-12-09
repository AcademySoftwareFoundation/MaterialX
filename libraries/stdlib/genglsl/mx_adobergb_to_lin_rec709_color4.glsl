#include "lib/mx_transform_color.glsl"

void mx_adobergb_to_lin_rec709_color4(vec4 _in, out vec4 result)
{
    vec3 linearColor = pow(max(_in.rgb, vec3(0.0)), vec3(ADOBERGB_GAMMA));
    result = vec4(ADOBERGB_TO_REC709 * linearColor, _in.a);
}
