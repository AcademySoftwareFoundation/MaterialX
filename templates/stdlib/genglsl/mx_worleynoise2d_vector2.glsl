#include "lib/mx_noise.glsl"

void mx_worleynoise2d_vector2(vec2 texcoord, float jitter, int style, out vec2 result)
{
    result = mx_worley_noise_vec2(texcoord, jitter, style, 0);
}
