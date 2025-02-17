struct MetalTextureArray
{
    texture2d_array<float> texArray;
    sampler s;
};

float4 texture(MetalTextureArray mtlTex, float2 uv, int index)
{
    float4 ret = float4(0, 0, 1, 0);

    if (index >= -0.5 && !is_null_texture(mtlTex.texArray)) {
        ret = vec4(mtlTex.texArray.sample(mtlTex.s, uv, index));
    }

    return ret;
}

float4 textureLod(MetalTextureArray mtlTex, float2 uv, int index, float lod)
{
    float4 ret = float4(0, 0, 0, 0);

    if (index >= -0.5  && !is_null_texture(mtlTex.texArray)) {
        ret = vec4(mtlTex.texArray.sample(mtlTex.s, uv, index, level(lod)));
    }

    return ret;
}

int2 textureSize(MetalTextureArray mtlTex, int mipLevel)
{
    return int2(mtlTex.texArray.get_width(), mtlTex.texArray.get_height());
}
