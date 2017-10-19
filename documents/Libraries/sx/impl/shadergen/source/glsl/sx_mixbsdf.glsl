void sx_mixbsdf(vec3 L, vec3 V, BSDF in1, float weight1, BSDF in2, float weight2, out BSDF result)
{
    float s = weight1 + weight2;
    if (s > 1.0)
    {
        s = 1.0 / s;
        weight1 *= s;
        weight2 *= s;
    }
    result.fr = in1.fr * weight1 + in2.fr * weight2;
    result.ft = in1.ft * weight1 + in2.ft * weight2;
}
