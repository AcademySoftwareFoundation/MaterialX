#include "stdlib/sx-glsl/hsv.glsl"

void mx_hsvtorgb_color4(vec4 _in, out vec4 result)
{
    result = vec4(hsvtorgb(_in.rgb), 1.0);
}
