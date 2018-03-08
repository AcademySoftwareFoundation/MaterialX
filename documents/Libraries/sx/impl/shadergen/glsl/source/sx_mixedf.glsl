void sx_mixedf(vec3 N, vec3 L, EDF fg, EDF bg, float mask, out EDF result)
{
    float weight = clamp(mask, 0.0, 1.0);
    result = fg * weight + bg * (1.0 - weight);
}
