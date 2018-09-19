void sx_roughness_vector2(float roughness, float anisotropy, out vec2 result)
{
    float roughness_sqr = roughness * roughness;
    result.x = roughness_sqr;
    result.y = roughness_sqr;
    if (anisotropy > 0.0)
    {
        float aspect = sqrt(1.0 - clamp(anisotropy, 0.0, 0.98));
        result.x = min(roughness_sqr / aspect, 1.0);
        result.y = roughness_sqr * aspect;
    }
}
