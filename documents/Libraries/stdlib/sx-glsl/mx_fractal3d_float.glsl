#include "stdlib/sx-glsl/libnoise.glsl"

void mx_fractal3d_float(float amplitude, int octaves, float lacunarity, float diminish, vec3 position, out float result)
{
    float value = fractal_noice1(position, octaves, lacunarity, diminish);
    result = value * amplitude;
}
