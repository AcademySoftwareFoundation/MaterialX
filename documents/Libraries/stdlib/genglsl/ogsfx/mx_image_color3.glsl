void mx_image_color3(sampler2D tex_sampler, int layer, vec3 defaultval, vec2 texcoord, int uaddressmode, int vaddressmode, int filtertype, int framerange, int frameoffset, int frameendaction, out vec3 result)
{
    // TODO: Fix handling of addressmode
    if(textureSize(tex_sampler, 0).x > 1)
    {
        // Flip v-component to match MaterialX convention
        texcoord.y = 1.0 - texcoord.y;
        result = texture(tex_sampler, texcoord).rgb;
    }
    else
    {
        result = defaultval;
    }
}
