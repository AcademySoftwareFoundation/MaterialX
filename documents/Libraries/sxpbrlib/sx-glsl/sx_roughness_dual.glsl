void sx_roughness_dual(float roughnessX, float roughnessY, out roughnessinfo result)
{
    if (roughnessY < 0.0)
    {
        result.roughness = roughnessX;
        result.alpha = clamp(roughnessX * roughnessX, M_FLOAT_EPS, 1.0);
        result.alphaX = result.alpha;
        result.alphaY = result.alpha;
    }
    else
    {
        float rx2 = roughnessX * roughnessX;
        float ry2 = roughnessY * roughnessY;
        result.roughness = sqrt(rx2 + ry2);
        result.alpha = clamp(result.roughness * result.roughness, M_FLOAT_EPS, 1.0);
        result.alphaX = clamp(rx2, M_FLOAT_EPS, 1.0);
        result.alphaY = clamp(ry2, M_FLOAT_EPS, 1.0);
    }
}
