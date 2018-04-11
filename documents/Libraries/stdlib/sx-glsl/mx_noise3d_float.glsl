#include "stdlib/sx-glsl/libnoise.glsl"

void mx_noise3d_float(float amplitude, float pivot, vec3 position, out float result)
{
    float value = perlin_noise1(position.x, position.y, position.z);
    result = value * amplitude + pivot;
}
