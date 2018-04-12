#include "stdlib/sx-glsl/lib/sxnoise.glsl"

void mx_cellnoise2d_float(vec2 texcoord, out float result)
{
    result = sx_cell_noise_float(texcoord.x, texcoord.y);
}
