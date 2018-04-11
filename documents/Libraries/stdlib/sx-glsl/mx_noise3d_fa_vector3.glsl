#include "stdlib/sx-glsl/libnoise.glsl"

void mx_noise3d_fa_vector3(float amplitude, float pivot, vec3 position, out vec3 result)
{
    vec3 value = perlin_noise3(position.x, position.y, position.z);
    result = value * amplitude + pivot;
}
