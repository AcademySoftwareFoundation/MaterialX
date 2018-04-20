void sx_refractionbsdf(vec3 L, vec3 V, vec3 transmittance, float ior, float roughness, float anisotropy, vec3 normal, vec3 tangent, int distribution, VDF interior, out BSDF result)
{
    // TODO: Attenuate the transmission based on roughness and VDF
    normal = sx_front_facing(normal);
    result.fr = vec3(0.0);
    result.ft = transmittance;
}
