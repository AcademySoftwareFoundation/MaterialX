void mx_roughness_dual(float roughness_x, float roughness_y, out roughnessinfo result)
{
    if (roughness_y < 0.0)
    {
        result.roughness = roughness_x;
        result.alpha = clamp(roughness_x * roughness_x, M_FLOAT_EPS, 1.0);
        result.alphaX = result.alpha;
        result.alphaY = result.alpha;
    }
    else
    {
        float rx2 = roughness_x * roughness_x;
        float ry2 = roughness_y * roughness_y;
        result.roughness = sqrt(rx2 + ry2);
        result.alpha = clamp(result.roughness * result.roughness, M_FLOAT_EPS, 1.0);
        result.alphaX = clamp(rx2, M_FLOAT_EPS, 1.0);
        result.alphaY = clamp(ry2, M_FLOAT_EPS, 1.0);
    }
}
