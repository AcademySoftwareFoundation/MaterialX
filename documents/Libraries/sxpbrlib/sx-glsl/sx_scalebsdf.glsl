void sx_scalebsdf_reflection(vec3 L, vec3 V, BSDF _in, vec3 weight, out BSDF result)
{
    weight = clamp(weight, 0.0, 1.0);
    result = _in * weight;
}

void sx_scalebsdf_transmission(vec3 V, BSDF _in, vec3 weight, out BSDF result)
{
    weight = clamp(weight, 0.0, 1.0);
    result = _in * weight;
}

void sx_scalebsdf_indirect(vec3 V, vec3 _in, vec3 weight, out vec3 result)
{
    weight = clamp(weight, 0.0, 1.0);
    result = _in * weight;
}
