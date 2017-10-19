void sx_mixbsdf(vec3 L, vec3 V, BSDF in1, BSDF in2, float weight, out BSDF result)
{
    result.fr = in1.fr * weight + in2.fr * (1.0 - weight);
    result.ft = in1.ft * weight + in2.ft * (1.0 - weight);
}
