void sx_translucentbsdf(vec3 L, vec3 V, vec3 transmittance, vec3 normal, out BSDF result)
{
    // Invert normal since we're transmitting light from the other side
    float cosTheta = max(dot(-normal, L), 0.0);
    result = transmittance * M_PI_INV * cosTheta;
}

void sx_translucentbsdf_ibl(vec3 V, vec3 transmittance, vec3 normal, out vec3 result)
{
    // Invert normal since we're transmitting light from the other side
    vec3 Li = sx_environment_irradiance(-normal);
    result = Li * transmittance;
}
