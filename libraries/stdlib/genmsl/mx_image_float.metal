#include "../genglsl/lib/$fileTransformUv"
#include "lib/mx_image_address.metal"

void mx_image_float($texSamplerSignature, int layer, float defaultval, vec2 texcoord, int uaddressmode, int vaddressmode, int filtertype, int framerange, int frameoffset, int frameendaction, vec2 uv_scale, vec2 uv_offset, out float result)
{
    vec2 uv = mx_transform_uv(texcoord, uv_scale, uv_offset);
    result = mx_image_coord_out_of_bounds(uv, uaddressmode, vaddressmode) ? defaultval : texture($texSamplerSampler2D, uv).r;
}
