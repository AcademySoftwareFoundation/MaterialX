#include "lib/mx_closure_type.glsl"

void mx_translucent_bsdf(ClosureData closureData, float weight, vec3 color, vec3 N, inout BSDF bsdf)
{
    // As an opaque BSDF, transmit light only through the fraction of the surface not covered by the weight.
    bsdf.throughput = vec3(1.0 - weight);

    if (weight < M_FLOAT_EPS)
    {
        return;
    }

    vec3 V = closureData.V;
    vec3 L = closureData.L;

    // Invert normal since we're transmitting light from the other side
    N = -N;

    if (closureData.closureType == CLOSURE_TYPE_REFLECTION)
    {
        float NdotL = clamp(dot(N, L), 0.0, 1.0);
        bsdf.response = color * weight * NdotL * M_PI_INV;
    }
    else if (closureData.closureType == CLOSURE_TYPE_INDIRECT)
    {
        vec3 Li = mx_environment_irradiance(N);
        bsdf.response = Li * color * weight;
    }
}
