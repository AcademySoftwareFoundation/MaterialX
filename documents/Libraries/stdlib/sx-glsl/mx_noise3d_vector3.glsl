#include "stdlib/sx-glsl/lib/sxnoise.glsl"

void mx_noise3d_vector3(vec3 amplitude, float pivot, vec3 position, out vec3 result)
{
    vec3 value = sx_perlin_noise_vec3(position);
    result = value * amplitude + pivot;
}
