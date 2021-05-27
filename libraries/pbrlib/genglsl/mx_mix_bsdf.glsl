void mx_mix_bsdf_reflection(vec3 L, vec3 V, vec3 P, float occlusion, BSDF fg, BSDF bg, float mix_weight, out BSDF bsdf)
{
    float w = clamp(mix_weight, 0.0, 1.0);
    bsdf.result = mix(bg.result, fg.result, w);
    bsdf.throughput = mix(bg.throughput, fg.throughput, w);
}

void mx_mix_bsdf_transmission(vec3 V, BSDF fg, BSDF bg, float mix_weight, out BSDF bsdf)
{
    float w = clamp(mix_weight, 0.0, 1.0);
    bsdf.result = mix(bg.result, fg.result, w);
    bsdf.throughput = mix(bg.throughput, fg.throughput, w);
}

void mx_mix_bsdf_indirect(vec3 V, BSDF fg, BSDF bg, float mix_weight, out BSDF bsdf)
{
    float w = clamp(mix_weight, 0.0, 1.0);
    bsdf.result = mix(bg.result, fg.result, w);
    bsdf.throughput = mix(bg.throughput, fg.throughput, w);
}
