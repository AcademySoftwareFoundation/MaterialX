#include "stdlib/sx-glsl/hsv.glsl"

void mx_hueshift_color3(vec3 _in, float amount, out vec3 result)
{
    vec3 hsv = rgb2hsv(_in);
    hsv.x += amount;
    result = hsv2rgb(hsv);
}
