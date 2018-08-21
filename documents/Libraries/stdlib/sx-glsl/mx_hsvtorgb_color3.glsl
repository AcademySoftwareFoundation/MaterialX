#include "stdlib/sx-glsl/hsv.glsl"

void mx_hsvtorgb_color3(vec3 _in, out vec3 result)
{
    result = hsvtorgb(_in);
}
