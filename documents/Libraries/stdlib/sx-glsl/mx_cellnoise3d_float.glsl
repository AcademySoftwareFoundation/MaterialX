#include "stdlib/sx-glsl/libnoise.glsl"

void mx_cellnoise3d_float(vec3 position, out float result)
{
    result = cell_noise1(position);
}
