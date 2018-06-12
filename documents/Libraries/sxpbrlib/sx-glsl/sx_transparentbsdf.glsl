void sx_transparentbsdf(vec3 L, vec3 V, float weight, vec3 transmittance, out BSDF result)
{
    result = transmittance * weight;
}

void sx_transparentbsdf_ibl(vec3 V, float weight, vec3 transmittance, out vec3 result)
{
    result = vec3(0.0);
}
