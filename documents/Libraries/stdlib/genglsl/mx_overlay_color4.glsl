#include "stdlib/genglsl/mx_overlay.glsl"

void mx_overlay_color4(vec4 fg, vec4 bg, float mix, out vec4 result)
{
    result = mix * mx_overlay(fg, bg) + (1-mix) * bg;
}
