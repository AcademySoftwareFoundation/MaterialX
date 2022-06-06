#include "lib/mx_transform_color.glsl"

void mx_srgb_texture_to_lin_rec709_color4(vec4 _in, out vec4 result)
{
    result = vec4(mx_srgb_texture_to_lin_rec709(_in.rgb), _in.a);
}
