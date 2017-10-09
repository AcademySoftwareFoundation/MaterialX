float sx_ggx_D(float cosTheta, float alpha)
{
    float a2 = square(alpha);
    return a2 / (M_PI * square( square(cosTheta) * (a2 - 1) + 1 ) );
}

float sx_smith_G1(float cosTheta, float alpha)
{
    float a2 = square(alpha);
    return 2/(1 + sqrt(1 + a2 * (1 - square(cosTheta)) / square(cosTheta) ));
}

float sx_smith_G(float NdotL, float NdotV, float alpha)
{
    return sx_smith_G1(NdotL, alpha) * sx_smith_G1(NdotV, alpha);
}

float sx_fresnel_schlick_roughness(float cosTheta, float ior, float roughness)
{
    float F0 = (ior - 1.0) / (ior + 1.0);
    F0 *= F0;
    return F0 + (max(1.0 - roughness, F0) - F0) * pow(1.0 - cosTheta, 5.0);
}
