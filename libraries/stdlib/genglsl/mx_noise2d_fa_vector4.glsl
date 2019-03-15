#include "stdlib/genglsl/lib/mx_noise.glsl"

void mx_noise2d_fa_vector4(float amplitude, float pivot, vec2 texcoord, out vec4 result)
{
    vec3 xyz = mx_perlin_noise_vec3(texcoord);
    float w = mx_perlin_noise_float(texcoord + vec2(19, 73));
    result = vec4(xyz, w) * amplitude + pivot;
}
