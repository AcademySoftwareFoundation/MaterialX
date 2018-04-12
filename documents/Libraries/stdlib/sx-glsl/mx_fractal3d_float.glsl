#include "stdlib/sx-glsl/lib/sxnoise.glsl"

void mx_fractal3d_float(float amplitude, int octaves, float lacunarity, float diminish, vec3 position, out float result)
{
    float value = sx_fractal_noice_float(position, octaves, lacunarity, diminish);
    result = value * amplitude;
}
