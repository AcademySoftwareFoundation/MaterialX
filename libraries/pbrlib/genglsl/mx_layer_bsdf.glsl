void mx_layer_bsdf_reflection(vec3 L, vec3 V, vec3 P, float occlusion, BSDF top, BSDF base, out BSDF bsdf)
{
    bsdf.eval = top.eval + top.throughput * base.eval;
    bsdf.throughput = top.throughput * base.throughput;
}

void mx_layer_bsdf_transmission(vec3 V, BSDF top, BSDF base, out BSDF bsdf)
{
    bsdf.eval = top.eval + top.throughput * base.eval;
    bsdf.throughput = top.throughput * base.throughput;
}

void mx_layer_bsdf_indirect(vec3 V, BSDF top, BSDF base, out BSDF bsdf)
{
    bsdf.eval = top.eval + top.throughput * base.eval;
    bsdf.throughput = top.throughput * base.throughput;
}
