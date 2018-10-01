#include "stdlib/sx-glsl/overlay.glsl"

void mx_overlay_color2(vec2 fg, vec2 bg, float mix, out vec2 result)
{
    result = mix * overlay(fg, bg) + (1-mix) * bg;
}
