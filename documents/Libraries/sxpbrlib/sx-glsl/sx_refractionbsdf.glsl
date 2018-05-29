void sx_refractionbsdf(vec3 L, vec3 V, vec3 transmittance, float ior, float roughness, float anisotropy, vec3 normal, vec3 tangent, int distribution, VDF interior, out BSDF result)
{
    // TODO: Attenuate the transmission based on roughness and interior VDF
    result = transmittance;
}

void sx_refractionbsdf_ibl(vec3 V, vec3 transmittance, float ior, float roughness, float anisotropy, vec3 normal, vec3 tangent, int distribution, VDF interior, out vec3 result)
{
    result = vec3(0.0);
}
