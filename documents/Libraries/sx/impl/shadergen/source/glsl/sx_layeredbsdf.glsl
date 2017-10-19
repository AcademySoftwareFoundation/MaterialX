void sx_layeredbsdf(vec3 L, vec3 V, BSDF top, BSDF base, float weight, out BSDF result)
{
    weight = saturate(weight);
    result.fr = top.fr * weight + base.fr * (1.0 - weight);
    result.ft = top.ft * weight + base.ft * (1.0 - weight);
}
