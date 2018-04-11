#include "stdlib/sx-glsl/libnoise.glsl"

void mx_noise2d_fa_vector4(float amplitude, float pivot, vec2 texcoord, out vec4 result)
{
    vec3 xyz = perlin_noise3(texcoord.x, texcoord.y);
    float w = perlin_noise1(texcoord.x + 19, texcoord.y + 73);
    result = vec4(xyz, w) * amplitude + pivot;
}
