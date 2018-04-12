#include "stdlib/sx-glsl/libnoise.glsl"

void mx_cellnoise2d_float(vec2 texcoord, out float result)
{
    result = cell_noise1(texcoord.x, texcoord.y);
}
