void mx_mix_edf(ClosureData closureData, EDF fg, EDF bg, float mixValue, out EDF result)
{
    result = mix(bg, fg, mixValue);
}
