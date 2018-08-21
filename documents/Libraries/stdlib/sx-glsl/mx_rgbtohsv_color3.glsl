#include "stdlib/sx-glsl/hsv.glsl"

void mx_rgb2hsv_color3(vec3 _in, out vec3 result)
{
    result = rgb2hsv(_in);
}
