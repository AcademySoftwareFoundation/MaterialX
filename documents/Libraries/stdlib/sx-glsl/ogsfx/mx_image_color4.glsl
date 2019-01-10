void mx_image_color4(sampler2D tex_sampler, int layer, vec4 defaultval, vec2 texcoord, int uaddressmode, int vaddressmode, int filtertype, int framerange, int frameoffset, int frameendaction, out vec4 result)
{
    // TODO: Fix handling of addressmode
    if(textureSize(tex_sampler, 0).x > 1)
    {
        // Flip v-component to match MaterialX convention
        texcoord.y = 1.0 - texcoord.y;
        result = texture(tex_sampler, texcoord);
    }
    else
    {
        result = defaultval;
    }
}
