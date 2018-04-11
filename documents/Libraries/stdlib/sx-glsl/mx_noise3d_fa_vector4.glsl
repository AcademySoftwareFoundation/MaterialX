#include "stdlib/sx-glsl/libnoise.glsl"

void mx_noise3d_fa_vector4(float amplitude, float pivot, vec3 position, out vec4 result)
{
    vec3 xyz = perlin_noise3(position.x, position.y, position.z);
    float w = perlin_noise1(position.x + 19, position.y + 73, position.z + 29);
    result = vec4(xyz, w) * amplitude + pivot;
}
