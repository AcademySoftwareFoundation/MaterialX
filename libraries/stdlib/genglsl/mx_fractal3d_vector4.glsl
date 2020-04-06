#include "stdlib/genglsl/lib/mx_noise.glsl"

void mx_fractal3d_vector4(vec4 amplitude, int octaves, float lacunarity, float diminish, vec3 position, out vec4 result)
{
    vec3 xyz = mx_fractal_noise_vec3(position, octaves, lacunarity, diminish);
    float w = mx_fractal_noise_float(position + vec3(19, 193, 17), octaves, lacunarity, diminish);
    result = vec4(xyz,w) * amplitude;
}
