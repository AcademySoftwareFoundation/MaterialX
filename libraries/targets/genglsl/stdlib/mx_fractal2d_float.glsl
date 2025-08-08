#include "lib/mx_noise.glsl"

void mx_fractal2d_float(float amplitude, int octaves, float lacunarity, float diminish, vec2 texcoord, out float result)
{
    float value = mx_fractal2d_noise_float(texcoord, octaves, lacunarity, diminish);
    result = value * amplitude;
}
