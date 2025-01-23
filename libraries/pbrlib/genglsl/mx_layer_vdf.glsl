void mx_layer_vdf(BSDF top, BSDF base, out BSDF result)
{
    result.response = top.response + base.response;
    result.throughput = top.throughput + base.throughput;
}

void mx_layer_vdf_reflection(vec3 _unused1, vec3 _unused2, vec3 _unused3, float weight, BSDF top, BSDF base, out BSDF result)
{
    mx_layer_vdf(top, base, result);
}

void mx_layer_vdf_transmission(vec3 _unused1, BSDF top, BSDF base, out BSDF result)
{
    mx_layer_vdf(top, base, result);
}

void mx_layer_vdf_indirect(vec3 _unused1, BSDF top, BSDF base, out BSDF result)
{
    mx_layer_vdf(top, base, result);
}
