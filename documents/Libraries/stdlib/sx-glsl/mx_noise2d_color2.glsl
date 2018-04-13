#include "stdlib/sx-glsl/sx_bits_to_01"

void mx_noise2d_color2(vec2 amplitude, float pivot, vec2 texcoord, out vec2 result)
{
    vec3 value = sx_perlin_noise_vec3(texcoord);
    result = value.xy * amplitude + pivot;
}
