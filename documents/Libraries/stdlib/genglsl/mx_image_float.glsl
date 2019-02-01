void mx_image_float(sampler2D tex_sampler, int layer, float defaultval, vec2 texcoord, int uaddressmode, int vaddressmode, int filtertype, int framerange, int frameoffset, int frameendaction, out float result)
{
    // TODO: Fix handling of addressmode
    if(textureSize(tex_sampler, 0).x > 1)
    {
        vec2 uv = mx_get_target_uv(texcoord);
        result = texture(tex_sampler, uv).r;
    }
    else
    {
        result = defaultval;
    }
}
