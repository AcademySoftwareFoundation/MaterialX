void mx_multiply_bsdf_color_reflection(vec3 L, vec3 V, vec3 P, float occlusion, BSDF in1, vec3 in2, out BSDF bsdf)
{
    vec3 w = clamp(in2, 0.0, 1.0);
    bsdf.eval = in1.eval * w;
    bsdf.throughput = in1.throughput * w;
}

void mx_multiply_bsdf_color_transmission(vec3 V, BSDF in1, vec3 in2, out BSDF bsdf)
{
    vec3 w = clamp(in2, 0.0, 1.0);
    bsdf.eval = in1.eval * w;
    bsdf.throughput = in1.throughput * w;
}

void mx_multiply_bsdf_color_indirect(vec3 V, BSDF in1, vec3 in2, out BSDF bsdf)
{
    vec3 w = clamp(in2, 0.0, 1.0);
    bsdf.eval = in1.eval * w;
    bsdf.throughput = in1.throughput * w;
}
