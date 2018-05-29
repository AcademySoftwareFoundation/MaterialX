void sx_scalebsdf(vec3 L, vec3 V, BSDF bsdf, vec3 weight, out BSDF result)
{
    weight = clamp(weight, 0.0, 1.0);
    result = bsdf * weight;
}

void sx_scalebsdf_ibl(vec3 V, BSDF bsdf, vec3 weight, out vec3 result)
{
    weight = clamp(weight, 0.0, 1.0);
    result = bsdf * weight;
}
