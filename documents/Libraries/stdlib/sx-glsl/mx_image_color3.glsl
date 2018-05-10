void mx_image_color3(sampler2D tex_sampler, int layer, vec3 defaultval, vec2 texcoord, int uaddressmode, int vaddressmode, int framerange, int frameoffset, int frameendaction, out vec3 result)
{
    // TODO: Fix handling of addressmode
    if(textureSize(tex_sampler, 0).x > 1)
    {
        vdirection(texcoord, texcoord);
        result = texture(tex_sampler, texcoord).rgb;
    }
    else
    {
        result = defaultval;
    }
}
