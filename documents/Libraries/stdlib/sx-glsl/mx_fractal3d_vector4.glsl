#include "stdlib/sx-glsl/libnoise.glsl"

void mx_fractal3d_vector4(vec4 amplitude, int octaves, float lacunarity, float diminish, vec3 position, out vec4 result)
{
    vec3 xyz = fractal_noice3(position, octaves, lacunarity, diminish);
    float w = fractal_noice1(position + vec3(19, 193, 17), octaves, lacunarity, diminish);
    result = vec4(xyz,w) * amplitude;
}
