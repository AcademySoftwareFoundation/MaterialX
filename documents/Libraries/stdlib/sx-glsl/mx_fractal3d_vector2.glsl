#include "stdlib/sx-glsl/lib/sxnoise.glsl"

void mx_fractal3d_vector2(vec2 amplitude, int octaves, float lacunarity, float diminish, vec3 position, out vec2 result)
{
    vec3 value = sx_fractal_noice_vec3(position, octaves, lacunarity, diminish);
    result = value.xy * amplitude;
}
