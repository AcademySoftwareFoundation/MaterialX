void sx_mixedf(vec3 N, vec3 L, EDF in1, EDF in2, float weight, out EDF result)
{
    weight = clamp(weight, 0.0, 1.0);
    result = in1 * weight + in2 * (1.0 - weight);
}
