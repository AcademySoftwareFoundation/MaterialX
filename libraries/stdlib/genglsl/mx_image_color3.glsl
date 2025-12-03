#include "lib/$fileTransformUv"

void mx_image_color3($texSamplerSignature, int layer, vec3 defaultval, vec2 texcoord, int uaddressmode, int vaddressmode, int filtertype, int framerange, int frameoffset, int frameendaction, vec2 uv_scale, vec2 uv_offset, out vec3 result)
{
    vec2 uv = mx_transform_uv(texcoord, uv_scale, uv_offset);

    bool outsideU = (uaddressmode == 0) && (uv.x < 0.0 || uv.x > 1.0);
    bool outsideV = (vaddressmode == 0) && (uv.y < 0.0 || uv.y > 1.0);
    bool useDefault = outsideU || outsideV;

    // Always sample once (avoids divergent control flow)
    vec3 sampled = texture($texSamplerSampler2D, uv).rgb;

    result = mix(sampled, defaultval, float(useDefault));
}
