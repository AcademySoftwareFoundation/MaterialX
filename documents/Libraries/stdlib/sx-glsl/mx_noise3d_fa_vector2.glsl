#include "stdlib/sx-glsl/lib/sxnoise.glsl"

void mx_noise3d_fa_vector2(float amplitude, float pivot, vec3 position, out vec2 result)
{
    vec3 value = sx_perlin_noise_vec3(position);
    result = value.xy * amplitude + pivot;
}
