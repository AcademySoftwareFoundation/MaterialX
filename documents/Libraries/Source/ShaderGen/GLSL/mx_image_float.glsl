void mx_image_float(sampler2D tex_sampler, int layer, float defaultval, vec2 texcoord, int uaddressmode, int vaddressmode, int framerange, int frameoffset, int frameendaction, out float result)
{
    // TODO: Fix handling of adressmode
    if(textureSize(tex_sampler, 0).x > 0)
    {
        vdirection(texcoord, texcoord);
        result = texture(tex_sampler, texcoord).r;
    }
    else
    {
        result = defaultval;
    }
}
