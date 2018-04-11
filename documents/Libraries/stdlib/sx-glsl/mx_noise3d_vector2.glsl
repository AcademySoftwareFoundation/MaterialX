#include "stdlib/sx-glsl/libnoise.glsl"

void mx_noise3d_vector2(vec2 amplitude, float pivot, vec3 position, out vec2 result)
{
    vec3 value = perlin_noise3(position.x, position.y, position.z);
    result = value.xy * amplitude + pivot;
}
