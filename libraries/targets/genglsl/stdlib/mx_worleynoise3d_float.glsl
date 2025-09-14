#include "lib/mx_noise.glsl"

void mx_worleynoise3d_float(vec3 position, float jitter, int style, out float result)
{
    result = mx_worley_noise_float(position, jitter, style, 0);
}
