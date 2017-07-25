#include "Source/ShaderGen/GLSL/hsv.glsl"

void mx_hueshift_color4(vec4 _in, float amount, out vec4 result)
{
    vec3 hsv = rgb2hsv(_in.rgb);
    hsv.x += amount;
    result.rgb = hsv2rgb(hsv);
    result.a = _in.a;
}
