#include "stdlib/genglsl/lib/mx_noise.glsl"

void mx_noise2d_fa_vector3(float amplitude, float pivot, vec2 texcoord, out vec3 result)
{
    vec3 value = mx_perlin_noise_vec3(texcoord);
    result = value * amplitude + pivot;
}
