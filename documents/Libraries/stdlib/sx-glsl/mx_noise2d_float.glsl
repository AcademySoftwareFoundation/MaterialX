#include "stdlib/sx-glsl/libnoise.glsl"

void mx_noise2d_float(float amplitude, float pivot, vec2 texcoord, out float result)
{
    float value = perlin_noise1(texcoord.x, texcoord.y);
    result = value * amplitude + pivot;
}
