void mx_absorption_vdf(vec3 absorption, inout BSDF bsdf)
{
    // TODO: Add some approximation for absorption.
    // float distance = clamp(abs(dot(N, V)), M_FLOAT_EPS, 1.0);
    // bsdf.throughput = bsdf.throughput * exp(-absorption * distance);
}
