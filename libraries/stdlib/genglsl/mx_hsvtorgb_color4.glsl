#include "lib/mx_hsv.glsl"

void mx_hsvtorgb_color4(vec4 _in, out vec4 result)
{
    result = vec4(mx_hsvtorgb(_in.rgb), _in.a);
}
