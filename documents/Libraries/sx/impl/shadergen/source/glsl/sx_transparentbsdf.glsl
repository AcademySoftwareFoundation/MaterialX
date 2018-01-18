void sx_transparentbsdf(vec3 L, vec3 V, vec3 transmittance, out BSDF result)
{
    result.fr = vec3(0.0);
    result.ft = transmittance;
}
