#include "pbrlib/genglsl/lib/mx_microfacet.glsl"

// https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf
// Appendix B.2 Equation 13
float mx_ggx_NDF(vec3 H, vec2 alpha)
{
    vec2 He = H.xy / alpha;
    float denom = dot(He, He) + mx_square(H.z);
    return 1.0 / (M_PI * alpha.x * alpha.y * mx_square(denom));
}

// https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf
// Appendix B.1 Equation 3
float mx_ggx_PDF(vec3 H, float LdotH, vec2 alpha)
{
    float NdotH = H.z;
    return mx_ggx_NDF(H, alpha) * NdotH / (4.0 * LdotH);
}

// https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf
// Appendix B.2 Equation 15
vec3 mx_ggx_importance_sample_NDF(vec2 Xi, vec2 alpha)
{
    float phi = 2.0 * M_PI * Xi.x;
    float tanTheta = sqrt(Xi.y / (1.0 - Xi.y));
    vec3 H = vec3(tanTheta * alpha.x * cos(phi),
                  tanTheta * alpha.y * sin(phi),
                  1.0);
    return normalize(H);
}

// http://jcgt.org/published/0007/04/01/paper.pdf
// Appendix A Listing 1
vec3 mx_ggx_importance_sample_VNDF(vec2 Xi, vec3 V, vec2 alpha)
{
    // Transform the view direction to the hemisphere configuration.
    V = normalize(vec3(V.xy * alpha, V.z));

    // Construct an orthonormal basis from the view direction.
    float len = length(V.xy);
    vec3 T1 = (len > 0.0) ? vec3(-V.y, V.x, 0.0) / len : vec3(1.0, 0.0, 0.0);
    vec3 T2 = cross(V, T1);

    // Parameterization of the projected area.
    float r = sqrt(Xi.y);
    float phi = 2.0 * M_PI * Xi.x;
    float t1 = r * cos(phi);
    float t2 = r * sin(phi);
    float s = 0.5 * (1.0 + V.z);
    t2 = (1.0 - s) * sqrt(1.0 - mx_square(t1)) + s * t2;

    // Reprojection onto hemisphere.
    vec3 H = t1 * T1 + t2 * T2 + sqrt(max(0.0, 1.0 - mx_square(t1) - mx_square(t2))) * V;

    // Transform the microfacet normal back to the ellipsoid configuration.
    H = normalize(vec3(H.xy * alpha, max(H.z, 0.0)));

    return H;
}

// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
// Equation 34
float mx_ggx_smith_G1(float cosTheta, float alpha)
{
    float cosTheta2 = mx_square(cosTheta);
    float tanTheta2 = (1.0 - cosTheta2) / cosTheta2;
    return 2.0 / (1.0 + sqrt(1.0 + mx_square(alpha) * tanTheta2));
}

// Height-correlated Smith masking-shadowing
// http://jcgt.org/published/0003/02/03/paper.pdf
// Equations 72 and 99
float mx_ggx_smith_G2(float NdotL, float NdotV, float alpha)
{
    float alpha2 = mx_square(alpha);
    float lambdaL = sqrt(alpha2 + (1.0 - alpha2) * mx_square(NdotL));
    float lambdaV = sqrt(alpha2 + (1.0 - alpha2) * mx_square(NdotV));
    return 2.0 / (lambdaL / NdotL + lambdaV / NdotV);
}

// Rational quadratic fit to Monte Carlo data for GGX directional albedo.
vec3 mx_ggx_dir_albedo_analytic(float NdotV, float roughness, vec3 F0, vec3 F90)
{
    float x = NdotV;
    float y = roughness;
    float x2 = mx_square(NdotV);
    float y2 = mx_square(roughness);
    vec4 r = vec4(0.10901, 0.92163, 1.0, 1.0) +
             vec4(-0.72019, -2.29412, -1.81963, -0.02130) * x +
             vec4(9.54750, 1.78914, 8.10489, 15.21516) * y +
             vec4(-0.93177, -2.62475, 12.89250, -45.75644) * x * y +
             vec4(29.60456, 1.40796, 29.11995, 13.53548) * x2 +
             vec4(-8.10263, -0.48479, -7.40042, 34.87345) * y2 +
             vec4(-27.96958, 0.71494, -36.69717, 26.50811) * x2 * y +
             vec4(18.28144, -0.50156, 13.79722, 253.01230) * x * y2 +
             vec4(-4.85579, 1.22080, 32.57057, -162.71012) * x2 * y2;
    vec2 AB = r.xy / r.zw;
    return F0 * AB.x + F90 * AB.y;
}

vec3 mx_ggx_dir_albedo_table_lookup(float NdotV, float roughness, vec3 F0, vec3 F90)
{
#if DIRECTIONAL_ALBEDO_METHOD == 1
    vec2 res = textureSize($albedoTable, 0);
    if (res.x > 1)
    {
        vec2 AB = texture($albedoTable, vec2(NdotV, roughness)).rg;
        return F0 * AB.x + F90 * AB.y;
    }
#endif
    return vec3(0.0);
}

// https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
vec3 mx_ggx_dir_albedo_monte_carlo(float NdotV, float roughness, vec3 F0, vec3 F90)
{
    NdotV = clamp(NdotV, M_FLOAT_EPS, 1.0);
    vec3 V = vec3(sqrt(1.0f - mx_square(NdotV)), 0, NdotV);

    vec2 AB = vec2(0.0);
    const int SAMPLE_COUNT = 64;
    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
        vec2 Xi = mx_spherical_fibonacci(i, SAMPLE_COUNT);

        // Compute the half vector and incoming light direction.
        vec3 H = mx_ggx_importance_sample_VNDF(Xi, V, vec2(roughness));
        vec3 L = -reflect(V, H);
        
        // Compute dot products for this sample.
        float NdotL = clamp(L.z, M_FLOAT_EPS, 1.0);
        float VdotH = clamp(dot(V, H), M_FLOAT_EPS, 1.0);

        // Compute the Fresnel term.
        float Fc = mx_fresnel_schlick(VdotH, 0.0, 1.0);

        // Compute the sample weight, combining the geometric term, BRDF denominator, and PDF.
        // https://hal.inria.fr/hal-00996995v2/document, Algorithm 2
        float weight = mx_ggx_smith_G2(NdotL, NdotV, roughness) / mx_ggx_smith_G1(NdotV, roughness);
        
        // Add the contribution of this sample.
        AB += vec2(weight * (1.0 - Fc), weight * Fc);
    }

    // Normalize integrated terms.
    AB /= float(SAMPLE_COUNT);

    // Return the final directional albedo.
    return F0 * AB.x + F90 * AB.y;
}

vec3 mx_ggx_dir_albedo(float NdotV, float roughness, vec3 F0, vec3 F90)
{
#if DIRECTIONAL_ALBEDO_METHOD == 0
    return mx_ggx_dir_albedo_analytic(NdotV, roughness, F0, F90);
#elif DIRECTIONAL_ALBEDO_METHOD == 1
    return mx_ggx_dir_albedo_table_lookup(NdotV, roughness, F0, F90);
#else
    return mx_ggx_dir_albedo_monte_carlo(NdotV, roughness, F0, F90);
#endif
}

float mx_ggx_dir_albedo(float NdotV, float roughness, float F0, float F90)
{
    return mx_ggx_dir_albedo(NdotV, roughness, vec3(F0), vec3(F90)).x;
}

// https://blog.selfshadow.com/publications/turquin/ms_comp_final.pdf
// Equations 14 and 16
vec3 mx_ggx_energy_compensation(float NdotV, float roughness, vec3 Fss)
{
    float Ess = mx_ggx_dir_albedo(NdotV, roughness, 1.0, 1.0);
    return 1.0 + Fss * (1.0 - Ess) / Ess;
}

float mx_ggx_energy_compensation(float NdotV, float roughness, float Fss)
{
    return mx_ggx_energy_compensation(NdotV, roughness, vec3(Fss)).x;
}

// Convert a real-valued index of refraction to normal-incidence reflectivity.
float mx_ior_to_f0(float ior)
{
    return mx_square((ior - 1.0) / (ior + 1.0));
}

// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
float mx_fresnel_dielectric(float cosTheta, float ior)
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

vec3 mx_fresnel_conductor(float cosTheta, vec3 n, vec3 k)
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

// Fresnel for dielectric/dielectric interface and polarized light.
void mx_fresnel_dielectric_polarized(float cosTheta, float n1, float n2, out vec2 F, out vec2 phi)
{
    float eta2 = mx_square(n1 / n2);
    float st2 = 1.0 - cosTheta*cosTheta;

    // Check for total internal reflection
    if(eta2*st2 > 1.0)
    {
        F = vec2(1.0);
        float s = sqrt(st2 - 1.0/eta2) / cosTheta;
        phi = 2.0 * atan(vec2(-eta2 * s, -s));
        return;
    }

    float cosTheta_t = sqrt(1.0 - eta2 * st2);
    vec2 r = vec2((n2*cosTheta - n1*cosTheta_t) / (n2*cosTheta + n1*cosTheta_t),
                  (n1*cosTheta - n2*cosTheta_t) / (n1*cosTheta + n2*cosTheta_t));
    F = mx_square(r);
    phi.x = (r.x < 0.0) ? M_PI : 0.0;
    phi.y = (r.y < 0.0) ? M_PI : 0.0;
}

// Fresnel for dielectric/conductor interface and polarized light.
// TODO: Optimize this functions and support wavelength dependent complex refraction index.
void mx_fresnel_conductor_polarized(float cosTheta, float n1, float n2, float k, out vec2 F, out vec2 phi)
{
    if (k == 0.0)
    {
        // Use dielectric formula to avoid numerical issues
        mx_fresnel_dielectric_polarized(cosTheta, n1, n2, F, phi);
        return;
    }

    float A = mx_square(n2) * (1.0 - mx_square(k)) - mx_square(n1) * (1.0 - mx_square(cosTheta));
    float B = sqrt(mx_square(A) + mx_square(2.0 * mx_square(n2) * k));
    float U = sqrt((A+B) / 2.0);
    float V = sqrt((B-A) / 2.0);

    F.y = (mx_square(n1*cosTheta - U) + mx_square(V)) / (mx_square(n1*cosTheta + U) + mx_square(V));
    phi.y = atan(2.0*n1 * V*cosTheta, mx_square(U) + mx_square(V) - mx_square(n1*cosTheta)) + M_PI;

    F.x = (mx_square(mx_square(n2) * (1.0 - mx_square(k)) * cosTheta - n1*U) + mx_square(2.0 * mx_square(n2) * k * cosTheta - n1*V)) /
            (mx_square(mx_square(n2) * (1.0 - mx_square(k)) * cosTheta + n1*U) + mx_square(2.0 * mx_square(n2) * k * cosTheta + n1*V));
    phi.x = atan(2.0 * n1 * mx_square(n2) * cosTheta * (2.0*k*U - (1.0 - mx_square(k)) * V), mx_square(mx_square(n2) * (1.0 + mx_square(k)) * cosTheta) - mx_square(n1) * (mx_square(U) + mx_square(V)));
}

// XYZ to CIE 1931 RGB color space (using neutral E illuminant)
const mat3 XYZ_TO_RGB = mat3(2.3706743, -0.5138850, 0.0052982, -0.9000405, 1.4253036, -0.0146949, -0.4706338, 0.0885814, 1.0093968);

// Depolarization functions for natural light
float mx_depolarize(vec2 v)
{
    return 0.5 * (v.x + v.y);
}
vec3 mx_depolarize(vec3 s, vec3 p)
{
    return 0.5 * (s + p);
}

// Evaluation XYZ sensitivity curves in Fourier space
vec3 mx_eval_sensitivity(float opd, float shift)
{
    // Use Gaussian fits, given by 3 parameters: val, pos and var
    float phase = 2.0*M_PI * opd;
    vec3 val = vec3(5.4856e-13, 4.4201e-13, 5.2481e-13);
    vec3 pos = vec3(1.6810e+06, 1.7953e+06, 2.2084e+06);
    vec3 var = vec3(4.3278e+09, 9.3046e+09, 6.6121e+09);
    vec3 xyz = val * sqrt(2.0*M_PI * var) * cos(pos * phase + shift) * exp(- var * phase*phase);
    xyz.x   += 9.7470e-14 * sqrt(2.0*M_PI * 4.5282e+09) * cos(2.2399e+06 * phase + shift) * exp(- 4.5282e+09 * phase*phase);
    return xyz / 1.0685e-7;
}

// A Practical Extension to Microfacet Theory for the Modeling of Varying Iridescence
// https://belcour.github.io/blog/research/2017/05/01/brdf-thin-film.html
vec3 mx_fresnel_airy(float cosTheta, vec3 ior, vec3 extinction, float tf_thickness, float tf_ior)
{
    // Convert nm -> m
    float d = tf_thickness * 1.0e-9;

    // Assume vacuum on the outside
    float eta1 = 1.0;
    float eta2 = tf_ior;

    // Optical path difference
    float cosTheta2 = sqrt(1.0 - mx_square(eta1/eta2) * (1.0 - mx_square(cosTheta)));
    float D = 2.0 * eta2 * d * cosTheta2;

    // First interface
    vec2 R12, phi12;
    mx_fresnel_dielectric_polarized(cosTheta, eta1, eta2, R12, phi12);
    vec2 R21  = R12;
    vec2 T121 = vec2(1.0) - R12;
    vec2 phi21 = vec2(M_PI) - phi12;

    // Second interface
    vec2 R23, phi23;
    mx_fresnel_conductor_polarized(cosTheta2, eta2, ior.x, extinction.x, R23, phi23);

    // Phase shift
    vec2 phi2 = phi21 + phi23;

    // Compound terms
    vec3 R = vec3(0.0);
    vec2 R123 = R12*R23;
    vec2 r123 = sqrt(R123);
    vec2 Rs   = mx_square(T121)*R23 / (1.0-R123);

    // Reflectance term for m=0 (DC term amplitude)
    vec2 C0 = R12 + Rs;
    vec3 S0 = mx_eval_sensitivity(0.0, 0.0);
    R += mx_depolarize(C0) * S0;

    // Reflectance term for m>0 (pairs of diracs)
    vec2 Cm = Rs - T121;
    for (int m=1; m<=3; ++m)
    {
        Cm *= r123;
        vec3 SmS = 2.0 * mx_eval_sensitivity(float(m)*D, float(m)*phi2.x);
        vec3 SmP = 2.0 * mx_eval_sensitivity(float(m)*D, float(m)*phi2.y);
        R += mx_depolarize(Cm.x*SmS, Cm.y*SmP);
    }

    // Convert back to RGB reflectance
    R = clamp(XYZ_TO_RGB * R, vec3(0.0), vec3(1.0));

    return R;
}

// Parameters for Fresnel calculations.
struct FresnelData
{
    vec3 ior;        // In Schlick Fresnel mode these two
    vec3 extinction; // hold F0 and F90 reflectance values
    float exponent;
    float tf_thickness;
    float tf_ior;
    int model;
};

FresnelData mx_init_fresnel_dielectric(float ior)
{
    FresnelData fd = FresnelData(vec3(0.0), vec3(0.0), 0.0, 0.0, 0.0, -1);
    fd.model = 0;
    fd.ior = vec3(ior);
    fd.tf_thickness = 0.0f;
    return fd;
}

FresnelData mx_init_fresnel_conductor(vec3 ior, vec3 extinction)
{
    FresnelData fd = FresnelData(vec3(0.0), vec3(0.0), 0.0, 0.0, 0.0, -1);
    fd.model = 1;
    fd.ior = ior;
    fd.extinction = extinction;
    fd.tf_thickness = 0.0f;
    return fd;
}

FresnelData mx_init_fresnel_schlick(vec3 F0)
{
    FresnelData fd = FresnelData(vec3(0.0), vec3(0.0), 0.0, 0.0, 0.0, -1);
    fd.model = 2;
    fd.ior = F0;
    fd.extinction = vec3(1.0);
    fd.exponent = 5.0f;
    fd.tf_thickness = 0.0f;
    return fd;
}

FresnelData mx_init_fresnel_schlick(vec3 F0, vec3 F90, float exponent)
{
    FresnelData fd = FresnelData(vec3(0.0), vec3(0.0), 0.0, 0.0, 0.0, -1);
    fd.model = 2;
    fd.ior = F0;
    fd.extinction = F90;
    fd.exponent = exponent;
    fd.tf_thickness = 0.0f;
    return fd;
}

FresnelData mx_init_fresnel_dielectric_airy(float ior, float tf_thickness, float tf_ior)
{
    FresnelData fd = FresnelData(vec3(0.0), vec3(0.0), 0.0, 0.0, 0.0, -1);
    fd.model = 3;
    fd.ior = vec3(ior);
    fd.extinction = vec3(0.0);
    fd.tf_thickness = tf_thickness;
    fd.tf_ior = tf_ior;
    return fd;
}

FresnelData mx_init_fresnel_conductor_airy(vec3 ior, vec3 extinction, float tf_thickness, float tf_ior)
{
    FresnelData fd = FresnelData(vec3(0.0), vec3(0.0), 0.0, 0.0, 0.0, -1);
    fd.model = 3;
    fd.ior = ior;
    fd.extinction = extinction;
    fd.tf_thickness = tf_thickness;
    fd.tf_ior = tf_ior;
    return fd;
}

vec3 mx_compute_fresnel(float cosTheta, FresnelData fd)
{
    if (fd.model == 0)
        return vec3(mx_fresnel_dielectric(cosTheta, fd.ior.x));
    else if (fd.model == 1)
        return mx_fresnel_conductor(cosTheta, fd.ior, fd.extinction);
    else if (fd.model == 2)
        // ior & extinction holds F0 & F90
        return mx_fresnel_schlick(cosTheta, fd.ior, fd.extinction, fd.exponent);
    else
        return mx_fresnel_airy(cosTheta, fd.ior, fd.extinction, fd.tf_thickness, fd.tf_ior);
}

vec2 mx_latlong_projection(vec3 dir)
{
    float latitude = -asin(dir.y) * M_PI_INV + 0.5;
    float longitude = atan(dir.x, -dir.z) * M_PI_INV * 0.5 + 0.5;
    return vec2(longitude, latitude);
}

vec3 mx_latlong_map_lookup(vec3 dir, mat4 transform, float lod, sampler2D envSampler)
{
    vec3 envDir = normalize((transform * vec4(dir,0.0)).xyz);
    vec2 uv = mx_latlong_projection(envDir);
    return textureLod(envSampler, uv, lod).rgb;
}
