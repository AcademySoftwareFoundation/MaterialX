#include "stdlib/genglsl/lib/mx_noise.glsl"

void mx_fractal3d_vector2(vec2 amplitude, int octaves, float lacunarity, float diminish, vec3 position, out vec2 result)
{
    vec3 value = mx_fractal_noise_vec3(position, octaves, lacunarity, diminish);
    result = value.xy * amplitude;
}
