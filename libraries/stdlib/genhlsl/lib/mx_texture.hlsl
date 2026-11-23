// Bundled (Texture, Sampler) pair so generated code can reference a single
// SamplerTexture2D handle (mirroring the Slang/Vulkan combined-image-sampler
// idiom). Host code is responsible for binding both members to the same slot
// pair.
struct SamplerTexture2D
{
    Texture2D tex;
    SamplerState samp;

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
};

float4 textureLod(SamplerTexture2D tex, float2 uv, float lod)
{
    return tex.tex.SampleLevel(tex.samp, uv, lod);
}

// HLSL reserves the bare identifier `texture` for the legacy D3DX Effects
// type, so we expose this entry point under a renamed function. The HLSL
// generator's GLSL->HLSL post-process rewrites every "texture(" call from the
// shared GLSL library into "mx_texture_sample(".
float4 mx_texture_sample(SamplerTexture2D tex, float2 uv)
{
    return tex.tex.Sample(tex.samp, uv);
}

float4 textureGrad(SamplerTexture2D tex, float2 uv, float2 ddx_, float2 ddy_)
{
    return tex.tex.SampleGrad(tex.samp, uv, ddx_, ddy_);
}

int2 textureSize(SamplerTexture2D tex, int mipLevel)
{
    return int2(tex.get_width(mipLevel), tex.get_height(mipLevel));
}
