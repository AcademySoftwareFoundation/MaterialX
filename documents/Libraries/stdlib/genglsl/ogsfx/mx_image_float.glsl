void mx_image_float(sampler2D tex_sampler, int layer, float defaultval, vec2 texcoord, int uaddressmode, int vaddressmode, int filtertype, int framerange, int frameoffset, int frameendaction, out float result)
{
    // TODO: Fix handling of addressmode
    if(textureSize(tex_sampler, 0).x > 1)
    {
        // Flip v-component to match MaterialX convention
        texcoord.y = 1.0 - texcoord.y;
        result = texture(tex_sampler, texcoord).r;
    }
    else
    {
        result = defaultval;
    }
}
