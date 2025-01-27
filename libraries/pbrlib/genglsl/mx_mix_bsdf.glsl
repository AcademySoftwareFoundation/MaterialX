void mx_mix_bsdf(BSDF fg, BSDF bg, float mixValue, out BSDF result)
{
    result.response = mix(bg.response, fg.response, mixValue);
    result.throughput = mix(bg.throughput, fg.throughput, mixValue);
}

void mx_mix_bsdf(vec3 _unused1, vec3 _unused2, BSDF fg, BSDF bg, float mix, out BSDF result)
{
    mx_mix_bsdf(fg, bg, mix, result);
}

void mx_mix_bsdf_reflection(vec3 _unused1, vec3 _unused2, vec3 _unused3, float weight, BSDF fg, BSDF bg, float mix, out BSDF result)
{
    mx_mix_bsdf(fg, bg, mix, result);
}

void mx_mix_bsdf_transmission(vec3 _unused1, BSDF fg, BSDF bg, float mix, out BSDF result)
{
    mx_mix_bsdf(fg, bg, mix, result);
}

void mx_mix_bsdf_indirect(vec3 _unused1, BSDF fg, BSDF bg, float mix, out BSDF result)
{
    mx_mix_bsdf(fg, bg, mix, result);
}
