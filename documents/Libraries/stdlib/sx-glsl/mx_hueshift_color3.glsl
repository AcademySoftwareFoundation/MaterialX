#include "stdlib/sx-glsl/hsv.glsl"

void mx_hueshift_color3(vec3 _in, float amount, out vec3 result)
{
    vec3 hsv = rgbtohsv(_in);
    hsv.x += amount;
    result = hsvtorgb(hsv);
}
