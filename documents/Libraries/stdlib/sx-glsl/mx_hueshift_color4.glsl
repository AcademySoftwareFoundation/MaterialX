#include "stdlib/sx-glsl/hsv.glsl"

void mx_hueshift_color4(vec4 _in, float amount, out vec4 result)
{
    vec3 hsv = rgbtohsv(_in.rgb);
    hsv.x += amount;
    result.rgb = hsvtorgb(hsv);
    result.a = _in.a;
}
