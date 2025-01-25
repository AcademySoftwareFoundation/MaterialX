void mx_mix_edf(vec3 N, vec3 L, EDF fg, EDF bg, float mixValue, out EDF result)
{
    result = mix(bg, fg, mixValue);
}
