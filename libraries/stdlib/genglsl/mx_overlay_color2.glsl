#include "stdlib/genglsl/mx_overlay.glsl"

void mx_overlay_color2(vec2 fg, vec2 bg, float mix, out vec2 result)
{
    result = mix * mx_overlay(fg, bg) + (1-mix) * bg;
}
