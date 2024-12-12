#include "lib/mx_noise.glsl"

void mx_worleynoise2d_float(vec2 texcoord, float jitter, int style, out float result)
{
    result = mx_worley_noise_float(texcoord, jitter, style, 0);
}
