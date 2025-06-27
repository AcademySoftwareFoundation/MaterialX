#include "lib/mx_noise.glsl"

void mx_fractal2d_vector4(vec4 amplitude, int octaves, float lacunarity, float diminish, vec2 texcoord, out vec4 result)
{
    vec4 value = mx_fractal2d_noise_vec4(texcoord, octaves, lacunarity, diminish);
    result = value * amplitude;
}
