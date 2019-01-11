void mx_mixedf(vec3 N, vec3 L, EDF in1, EDF in2, float weight, out EDF result)
{
    weight = clamp(weight, 0.0, 1.0);
    result = in1 * (1.0 - weight) + in2 * weight;
}
