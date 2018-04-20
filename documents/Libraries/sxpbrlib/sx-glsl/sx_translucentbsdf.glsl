void sx_translucentbsdf(vec3 L, vec3 V, vec3 transmittance, vec3 normal, out BSDF result)
{
    normal = sx_front_facing(normal);
    // Invert normal since we're transmitting light from the other side
    float cosTheta = max(dot(-normal, L), 0.0);
    result.fr = transmittance * M_PI_INV * cosTheta;
    result.ft = vec3(0.0);
}
