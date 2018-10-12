void sx_roughness(float roughness, float anisotropy, out roughnessinfo result)
{
    result.roughness = roughness;
    result.alpha = clamp(roughness*roughness, M_FLOAT_EPS, 1.0);
    if (anisotropy > 0.0)
    {
        float aspect = sqrt(1.0 - clamp(anisotropy, 0.0, 0.98));
        result.alphaX = min(result.alpha / aspect, 1.0);
        result.alphaY = result.alpha * aspect;
    }
    else
    {
        result.alphaX = result.alpha;
        result.alphaY = result.alpha;
    }
}
