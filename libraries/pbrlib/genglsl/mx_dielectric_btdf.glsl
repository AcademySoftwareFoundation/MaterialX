void mx_dielectric_btdf_transmission(vec3 V, float weight, vec3 tint, float ior, vec2 roughness, vec3 normal, vec3 tangent, int distribution, VDF interior, out BSDF result)
{
    // TODO: Attenuate the transmission based on roughness and interior VDF
    result = tint * weight;
}
