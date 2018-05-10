float sx_orennayar(vec3 L, vec3 V, vec3 N, float NdotL, float roughness)
{
    float LdotV = dot(L, V);
    float NdotV = dot(N, V);

    float t = LdotV - NdotL * NdotV;
    t = t > 0.0 ? t / max(NdotL, NdotV) : 0.0;

    float sigma2 = sx_square(roughness * M_PI);
    float A = 1.0 - 0.5 * (sigma2 / (sigma2 + 0.33));
    float B = 0.45f * sigma2 / (sigma2 + 0.09);

    return A + B * t;
}

// https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
// Appendix B.2 Equation 13
float sx_microfacet_ggx_NDF(vec3 X, vec3 Y, vec3 H, float NdotH, float alphaX, float alphaY)
{
    float XdotH = dot(X, H);
    float YdotH = dot(Y, H);
    float denom = XdotH * XdotH / (alphaX * alphaX) + YdotH * YdotH / (alphaY * alphaY) + NdotH * NdotH;
    return denom > 0.0 ? 1.0 / (M_PI * alphaX * alphaY * denom * denom) : 0.0;
}

// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf (Equation 34)
float sx_microfacet_ggx_G1(float cosTheta, float alpha)
{
    float cosTheta2 = cosTheta * cosTheta;
    float tanTheta2 = (1.0 - cosTheta2) / cosTheta2;
    return 2.0 / (1.0 + sqrt(1.0 + alpha * alpha * tanTheta2));
}

// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf (Equation 23)
float sx_microfacet_ggx_smith_G(float NdotL, float NdotV, float alpha)
{
    return sx_microfacet_ggx_G1(NdotL, alpha) * sx_microfacet_ggx_G1(NdotV, alpha);
}

float sx_fresnel_schlick(float cosTheta, float ior)
{
    if (cosTheta < 0.0)
        return 1.0;
    float F0 = (ior - 1.0) / (ior + 1.0);
    float m = 1.0 - cosTheta;
    return F0 + (1.0 - F0) * (m * m) * (m * m) * m;
}

float sx_fresnel_schlick_roughness(float cosTheta, float ior, float roughness)
{
    if (cosTheta < 0.0)
        return 1.0;
    float F0 = (ior - 1.0) / (ior + 1.0);
    F0 *= F0;
    float m = 1.0 - cosTheta;
    return F0 + (max(1.0 - roughness, F0) - F0) * (m * m) * (m * m) * m;
}
