#include "lib/mx_noise.glsl"

void mx_fractal2d_vector2(vec2 amplitude, int octaves, float lacunarity, float diminish, vec2 texcoord, out vec2 result)
{
    vec2 value = mx_fractal2d_noise_vec2(texcoord, octaves, lacunarity, diminish);
    result = value * amplitude;
}
