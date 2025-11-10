struct SamplerTexture2D
{
    Texture2D tex;
    SamplerState sampler;

    // needed for Storm
    int get_width(int mipLevel = 0)
    {
        uint width, height, numberOfLevels;
        tex.GetDimensions(mipLevel, width, height, numberOfLevels);
        return width;
    }

    int get_height(int mipLevel = 0)
    {
        uint width, height, numberOfLevels;
        tex.GetDimensions(mipLevel, width, height, numberOfLevels);
        return height;
    }

    int get_num_mip_levels()
    {
        uint width, height, numberOfLevels;
        tex.GetDimensions(0, width, height, numberOfLevels);
        return numberOfLevels;
    }
}

float4 textureLod(SamplerTexture2D tex, float2 uv, float lod)
{
    return tex.tex.SampleLevel(tex.sampler, uv, lod);
}

float4 texture(SamplerTexture2D tex, float2 uv)
{
    return tex.tex.Sample(tex.sampler, uv);
}

float4 textureGrad(SamplerTexture2D tex, float2 uv, float2 ddx_, float2 ddy_)
{
    return tex.tex.SampleGrad(tex.sampler, uv, ddx_, ddy_);
}

int2 textureSize(SamplerTexture2D tex, int mipLevel)
{
    return int2(tex.get_width(mipLevel), tex.get_height(mipLevel));
}
