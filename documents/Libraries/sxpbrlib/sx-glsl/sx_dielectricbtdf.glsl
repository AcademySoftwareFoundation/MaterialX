void sx_dielectricbtdf(vec3 L, vec3 V, float weight, vec3 color, float ior, vec2 roughness, vec3 normal, vec3 tangent, int distribution, VDF interior, out BSDF result)
{
    // TODO: Attenuate the transmission based on roughness and interior VDF
    result = color * weight;
}

void sx_dielectricbtdf_ibl(vec3 V, float weight, vec3 color, float ior, vec2 roughness, vec3 normal, vec3 tangent, int distribution, VDF interior, out vec3 result)
{
    result = vec3(0.0);
}
