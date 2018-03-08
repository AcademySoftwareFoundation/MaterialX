void sx_mixbsdf(vec3 L, vec3 V, BSDF fg, BSDF bg, float mask, out BSDF result)
{
    float weight = clamp(mask, 0.0, 1.0);
    result.fr = fg.fr * weight + bg.fr * (1.0 - weight);
    result.ft = fg.ft * weight + bg.ft * (1.0 - weight);
}
