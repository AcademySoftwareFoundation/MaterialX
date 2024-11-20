#include "lib/$fileTransformUv"

void mx_imagearray_vector3(sampler2DArray tex_sampler, int index, vec3 defaultval, vec2 texcoord, int uaddressmode, int vaddressmode, int filtertype, vec2 uv_scale, vec2 uv_offset, out vec3 result)
{
    vec2 uv = mx_transform_uv(texcoord, uv_scale, uv_offset);
    result = texture(tex_sampler, uv, index).rgb;
}
