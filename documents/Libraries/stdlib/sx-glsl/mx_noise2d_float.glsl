#include "stdlib/sx-glsl/lib/sxnoise.glsl"

void mx_noise2d_float(float amplitude, float pivot, vec2 texcoord, out float result)
{
    float value = sx_perlin_noise_float(texcoord);
    result = value * amplitude + pivot;
}
