#include "lib/mx_noise.glsl"

void mx_fractal3d_fa_vector4(float amplitude, int octaves, float lacunarity, float diminish, vec3 position, out vec4 result)
{
    vec4 value = mx_fractal_noise_vec4(position, octaves, lacunarity, diminish);
    result = value * amplitude;
}
