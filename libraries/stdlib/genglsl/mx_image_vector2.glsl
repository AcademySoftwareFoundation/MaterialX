#include "lib/$fileTransformUv"

#ifdef HW_SEPARATE_SAMPLERS
void mx_image_vector2(texture2D tex_texture, sampler tex_sampler, int layer, vec2 defaultval, vec2 texcoord, int uaddressmode, int vaddressmode, int filtertype, int framerange, int frameoffset, int frameendaction, vec2 uv_scale, vec2 uv_offset, out vec2 result)
{
    vec2 uv = mx_transform_uv(texcoord, uv_scale, uv_offset);
    result = texture(sampler2D(tex_texture, tex_sampler), uv).rg;
}
#else
void mx_image_vector2(sampler2D tex_sampler, int layer, vec2 defaultval, vec2 texcoord, int uaddressmode, int vaddressmode, int filtertype, int framerange, int frameoffset, int frameendaction, vec2 uv_scale, vec2 uv_offset, out vec2 result)
{
    vec2 uv = mx_transform_uv(texcoord, uv_scale, uv_offset);
    result = texture(tex_sampler, uv).rg;
}
#endif
