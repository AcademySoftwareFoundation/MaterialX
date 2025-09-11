#include "lib/mx_noise.glsl"

void mx_fractal2d_vector3(vec3 amplitude, int octaves, float lacunarity, float diminish, vec2 texcoord, out vec3 result)
{
    vec3 value = mx_fractal2d_noise_vec3(texcoord, octaves, lacunarity, diminish);
    result = value * amplitude;
}
