// Square functions for cleaner code
float sqr(float x) { return x*x; }
vec2 sqr(vec2 x) { return x*x; }
float maxv(vec3 v) { return max(max(v.x, v.y), v.z); }

float sx_ggx_D(float cosTheta, float alpha)
{
    float a2 = sqr(alpha);
    return a2 / (M_PI * sqr( sqr(cosTheta) * (a2 - 1) + 1 ) );
}

float sx_smith_G1(float cosTheta, float alpha)
{
    float a2 = sqr(alpha);
    return 2/(1 + sqrt(1 + a2 * (1-sqr(cosTheta)) / sqr(cosTheta) ));
}

float sx_smith_G(float NdotL, float NdotV, float alpha)
{
    return sx_smith_G1(NdotL, alpha) * sx_smith_G1(NdotV, alpha);
}

float sx_fresnel_schlick_roughness(float cosTheta, float Eta, float roughness)
{
    float F0 = (Eta - 1.0) / (Eta + 1.0);
    F0 *= F0;
    return F0 + (max(1.0 - roughness, F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

void sx_coatingbsdf(vec3 L, vec3 V, vec3 reflectance, float ior, float roughness, float anisotropy, vec3 normal, vec3 tangent, int distribution, BSDF base, out BSDF result)
{
    result = base;

    float NdotL = dot(normal,L);
    float NdotV = dot(normal,V);
    if (NdotL < 0 || NdotV < 0) 
        return;

    vec3 H = normalize(L+V);
    float NdotH = dot(normal,H);

    float alpha = roughness * roughness;

    float D = sx_ggx_D(NdotH, alpha);
    float G = sx_smith_G(NdotL, NdotV, alpha);
    float F = sx_fresnel_schlick_roughness(NdotH, ior, alpha);

    result.fr = reflectance * D * G * F * NdotH / (4*NdotV*NdotL) + base.fr * (1.0 - F);
    result.ft = vec3(0.0);
}
