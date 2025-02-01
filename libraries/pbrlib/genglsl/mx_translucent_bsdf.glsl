// We fake diffuse transmission by using diffuse reflection from the opposite side.
// So this BTDF is really a BRDF.
void mx_translucent_bsdf_reflection(vec3 L, vec3 V, vec3 P, float occlusion, float weight, vec3 color, vec3 normal, inout BSDF bsdf)
{
    bsdf.throughput = vec3(0.0);

    // Invert normal since we're transmitting light from the other side
    float NdotL = dot(L, -normal);
    if (NdotL <= 0.0 || weight < M_FLOAT_EPS)
    {
        return;
    }

    bsdf.response = color * weight * NdotL * M_PI_INV;
}

void mx_translucent_bsdf_indirect(vec3 V, float weight, vec3 color, vec3 normal, inout BSDF bsdf)
{
    bsdf.throughput = vec3(0.0);

    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    // Invert normal since we're transmitting light from the other side
    vec3 Li = mx_environment_irradiance(-normal);
    bsdf.response = Li * color * weight;
}

void mx_translucent_bsdf(ClosureData closureData, float weight, vec3 color, vec3 normal, inout BSDF bsdf)
{
    if (closureData.closureType == CLOSURE_TYPE_REFLECTION)
    {
        mx_translucent_bsdf_reflection(closureData.L, closureData.V, closureData.P, closureData.occlusion, weight, color, normal, bsdf);
    }
    else if (closureData.closureType == CLOSURE_TYPE_INDIRECT)
    {
        mx_translucent_bsdf_indirect(closureData.V, weight, color, normal, bsdf);
    }
}
