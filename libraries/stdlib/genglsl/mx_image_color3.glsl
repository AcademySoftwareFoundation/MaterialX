#include "lib/$fileTransformUv"

void mx_image_color3($texSamplerSignature, int layer, vec3 defaultval, vec2 texcoord, int uaddressmode, int vaddressmode, int filtertype, int framerange, int frameoffset, int frameendaction, vec2 uv_scale, vec2 uv_offset, out vec3 result)
{
    vec2 uv = mx_transform_uv(texcoord, uv_scale, uv_offset);

    // "constant" wrap mode directly implemented in shader source because MSL and GLSL implementations don't align
    if ((uaddressmode == 0 && (uv[0] < 0 || uv[0] > 1)) || (vaddressmode == 0 && (uv[1] < 0 || uv[1] > 1)))
    {
        result = defaultval;
        return;
    }

    result = texture($texSamplerSampler2D, uv).rgb;
}
