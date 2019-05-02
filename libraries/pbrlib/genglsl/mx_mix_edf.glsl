void mx_mix_edf(vec3 N, vec3 L, EDF fg, EDF bg, float w, out EDF result)
{
    result = mix(bg, fg, clamp(w, 0.0, 1.0));
}
