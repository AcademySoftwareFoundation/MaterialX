void mx_dielectricbtdf_transmission(vec3 V, float weight, vec3 tint, float ior, roughnessinfo roughness, vec3 normal, vec3 tangent, int distribution, VDF interior, out BSDF result)
{
    // TODO: Attenuate the transmission based on roughness and interior VDF
    result = tint * weight;
}
