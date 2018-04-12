#include "stdlib/sx-glsl/lib/sxnoise.glsl"

void mx_fractal3d_vector4(vec4 amplitude, int octaves, float lacunarity, float diminish, vec3 position, out vec4 result)
{
    vec3 xyz = sx_fractal_noice_vec3(position, octaves, lacunarity, diminish);
    float w = sx_fractal_noice_float(position + vec3(19, 193, 17), octaves, lacunarity, diminish);
    result = vec4(xyz,w) * amplitude;
}
