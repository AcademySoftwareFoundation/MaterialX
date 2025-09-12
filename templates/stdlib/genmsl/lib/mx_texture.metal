struct MetalTexture
{
    texture2d<float> tex;
    sampler s;

    // needed for Storm
    int get_width() { return tex.get_width(); }
    int get_height() { return tex.get_height(); }
    int get_num_mip_levels() { return tex.get_num_mip_levels(); }
};

float4 texture(MetalTexture mtlTex, float2 uv)
{
    return mtlTex.tex.sample(mtlTex.s, uv);
}

float4 textureLod(MetalTexture mtlTex, float2 uv, float lod)
{
    return mtlTex.tex.sample(mtlTex.s, uv, level(lod));
}

float4 textureGrad(MetalTexture mtlTex, float2 uv, float2 dx, float2 dy)
{
    return mtlTex.tex.sample(mtlTex.s, uv, gradient2d(dx, dy));
}

int2 textureSize(MetalTexture mtlTex, int mipLevel)
{
    return int2(mtlTex.tex.get_width(), mtlTex.tex.get_height());
}
