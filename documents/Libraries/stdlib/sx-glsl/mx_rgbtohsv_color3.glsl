#include "stdlib/sx-glsl/hsv.glsl"

void mx_rgbtohsv_color3(vec3 _in, out vec3 result)
{
    result = rgbtohsv(_in);
}
