void mx_mix_bsdf_reflection(vec3 L, vec3 V, BSDF fg, BSDF bg, float w, out BSDF result)
{
    result = mix(bg, fg, clamp(w, 0.0, 1.0));
}

void mx_mix_bsdf_transmission(vec3 V, BSDF fg, BSDF bg, float w, out BSDF result)
{
    result = mix(bg, fg, clamp(w, 0.0, 1.0));
}

void mx_mix_bsdf_indirect(vec3 V, vec3 fg, vec3 bg, float w, out vec3 result)
{
    result = mix(bg, fg, clamp(w, 0.0, 1.0));
}
