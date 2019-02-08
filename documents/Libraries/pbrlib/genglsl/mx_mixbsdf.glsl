void mx_mixbsdf_reflection(vec3 L, vec3 V, BSDF in1, BSDF in2, float weight, out BSDF result)
{
    weight = clamp(weight, 0.0, 1.0);
    result = in1 * (1.0 - weight) + in2 * weight;
}

void mx_mixbsdf_transmission(vec3 V, BSDF in1, BSDF in2, float weight, out BSDF result)
{
    weight = clamp(weight, 0.0, 1.0);
    result = in1 * (1.0 - weight) + in2 * weight;
}

void mx_mixbsdf_indirect(vec3 V, vec3 in1, vec3 in2, float weight, out vec3 result)
{
    weight = clamp(weight, 0.0, 1.0);
    result = in1 * (1.0 - weight) + in2 * weight;
}
