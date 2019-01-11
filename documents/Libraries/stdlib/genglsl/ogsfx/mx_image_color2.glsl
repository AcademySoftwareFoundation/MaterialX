void mx_image_color2(sampler2D tex_sampler, int layer, vec2 defaultval, vec2 texcoord, int uaddressmode, int vaddressmode, int filtertype, int framerange, int frameoffset, int frameendaction, out vec2 result)
{
    // TODO: Fix handling of addressmode
    if(textureSize(tex_sampler, 0).x > 1)
    {
        // Flip v-component to match MaterialX convention
        texcoord.y = 1.0 - texcoord.y;
        result = texture(tex_sampler, texcoord).rg;
    }
    else
    {
        result = defaultval;
    }
}
