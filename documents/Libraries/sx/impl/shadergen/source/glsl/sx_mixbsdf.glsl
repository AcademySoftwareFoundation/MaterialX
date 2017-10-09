void sx_mixbsdf(vec3 L, vec3 V, BSDF fg, BSDF bg, float mask, out BSDF result)
{
    result.fr = fg.fr * mask + bg.fr * (1.0 - mask);
    result.ft = fg.ft * mask + bg.ft * (1.0 - mask);
}
