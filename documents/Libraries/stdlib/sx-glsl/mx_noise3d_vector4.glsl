#include "stdlib/sx-glsl/lib/sxnoise.glsl"

void mx_noise3d_vector4(vec4 amplitude, float pivot, vec3 position, out vec4 result)
{
    vec3 xyz = sx_perlin_noise_vec3(position);
    float w = sx_perlin_noise_float(position + vec3(19, 73, 29));
    result = vec4(xyz, w) * amplitude + pivot;
}
