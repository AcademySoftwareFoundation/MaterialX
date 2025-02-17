#include "lib/$fileTransformUv"

void mx_imagearray_float(sampler2DArray tex_sampler, int index, float defaultval, vec2 texcoord, int uaddressmode, int vaddressmode, int filtertype, vec2 uv_scale, vec2 uv_offset, out float result)
{
    vec2 uv = mx_transform_uv(texcoord, uv_scale, uv_offset);
    result = texture(tex_sampler, uv, index).r;
}
