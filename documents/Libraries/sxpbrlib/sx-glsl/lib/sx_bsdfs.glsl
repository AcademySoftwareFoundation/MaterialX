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
    F0 *= F0;
    float x = 1.0 - cosTheta;
    float x2 = x*x;
    float x5 = x2*x2*x;
    return F0 + (1.0 - F0) * x5;
}

float sx_fresnel_schlick_roughness(float cosTheta, float ior, float roughness)
{
    cosTheta = abs(cosTheta);
    float F0 = (ior - 1.0) / (ior + 1.0);
    F0 *= F0;
    float x = 1.0 - cosTheta;
    float x2 = x*x;
    float x5 = x2*x2*x;
    return F0 + (max(1.0 - roughness, F0) - F0) * x5;
}

// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
float sx_fresnel_dielectric(float cosTheta, float ior)
{
    if (cosTheta < 0.0)
        return 1.0;

    float g =  ior*ior + cosTheta*cosTheta - 1.0;
    // Check for total internal reflection
    if (g < 0.0)
        return 1.0;

    g = sqrt(g);
    float gmc = g - cosTheta;
    float gpc = g + cosTheta;
    float x = gmc / gpc;
    float y = (gpc * cosTheta - 1.0) / (gmc * cosTheta + 1.0);
    return 0.5 * x * x * (1.0 + y * y);
}

vec3 sx_fresnel_conductor(float cosTheta, vec3 n, vec3 k)
{
   float c2 = cosTheta*cosTheta;
   vec3 n2_k2 = n*n + k*k;
   vec3 nc2 = 2.0 * n * cosTheta;

   vec3 rs_a = n2_k2 + c2;
   vec3 rp_a = n2_k2 * c2 + 1.0;
   vec3 rs = (rs_a - nc2) / (rs_a + nc2);
   vec3 rp = (rp_a - nc2) / (rp_a + nc2);

   return 0.5 * (rs + rp);
}
