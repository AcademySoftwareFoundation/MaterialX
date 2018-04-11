#include "stdlib/sx-glsl/libnoise.glsl"

void mx_noise2d_color2(vec2 amplitude, float pivot, vec2 texcoord, out vec2 result)
{
    vec3 value = perlin_noise3(texcoord.x, texcoord.y);
    result = value.xy * amplitude + pivot;
}
