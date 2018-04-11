#include "stdlib/sx-glsl/libnoise.glsl"

void mx_noise2d_vector3(vec3 amplitude, float pivot, vec2 texcoord, out vec3 result)
{
    vec3 value = perlin_noise3(texcoord.x, texcoord.y);
    result = value * amplitude + pivot;
}
