#include "stdlib/genglsl/lib/mx_noise.glsl"

void mx_noise3d_fa_vector3(float amplitude, float pivot, vec3 position, out vec3 result)
{
    vec3 value = mx_perlin_noise_vec3(position);
    result = value * amplitude + pivot;
}
