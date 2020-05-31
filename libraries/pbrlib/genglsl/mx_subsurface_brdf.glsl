// Fake with simple diffuse transmission
void mx_subsurface_brdf_reflection(vec3 L, vec3 V, float weight, vec3 color, vec3 radius, float anisotropy, vec3 normal, out BSDF result)
{
    // Invert normal since we're transmitting light from the other side
    float NdotL = dot(L, -normal);
    if (NdotL <= 0.0 || weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    result = color * weight * NdotL * M_PI_INV;
}

// Fake with simple diffuse transmission
void mx_subsurface_brdf_indirect(vec3 V, float weight, vec3 color, vec3 radius, float anisotropy, vec3 normal, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    // Invert normal since we're transmitting light from the other side
    vec3 Li = mx_environment_irradiance(-normal);
    result = Li * color * weight;
}
