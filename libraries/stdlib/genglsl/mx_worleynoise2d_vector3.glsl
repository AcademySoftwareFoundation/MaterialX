#include "lib/mx_noise.glsl"

void mx_worleynoise2d_vector3(vec2 texcoord, float jitter, int style, out vec3 result)
{
    result = mx_worley_noise_vec3(texcoord, jitter, style, 0);
}
