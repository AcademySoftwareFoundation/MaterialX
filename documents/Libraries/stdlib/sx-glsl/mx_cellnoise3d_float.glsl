#include "stdlib/sx-glsl/lib/sxnoise.glsl"

void mx_cellnoise3d_float(vec3 position, out float result)
{
    result = sx_cell_noise_float(position);
}
