void sx_translucentbsdf(vec3 L, vec3 V, float weight, vec3 transmittance, vec3 normal, out BSDF result)
{
    // Invert normal since we're transmitting light from the other side
    float NdotL = dot(L, -normal);
    if (NdotL <= 0.0 || weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    result = transmittance * weight * NdotL * M_PI_INV;
}

void sx_translucentbsdf_ibl(vec3 V, float weight, vec3 transmittance, vec3 normal, out vec3 result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = vec3(0.0);
        return;
    }

    // Invert normal since we're transmitting light from the other side
    vec3 Li = sx_environment_irradiance(-normal);
    result = Li * transmittance * weight;
}
