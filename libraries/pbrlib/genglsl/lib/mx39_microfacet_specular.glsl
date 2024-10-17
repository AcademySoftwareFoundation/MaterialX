#include "mx39_microfacet.glsl"
#include "mx_microfacet_specular.glsl"

const int MX39_FRESNEL_MODEL_DIELECTRIC = 0;
const int MX39_FRESNEL_MODEL_CONDUCTOR = 1;
const int MX39_FRESNEL_MODEL_SCHLICK = 2;

// Parameters for Fresnel calculations
struct Mx39FresnelData
{
    // Fresnel model
    int model;
    bool airy;

    // Physical Fresnel
    vec3 ior;
    vec3 extinction;

    // Generalized Schlick Fresnel
    vec3 F0;
    vec3 F82;
    vec3 F90;
    float exponent;

    // Thin film
    float tf_thickness;
    float tf_ior;

    // Refraction
    bool refraction;
};

// Convert a real-valued index of refraction to normal-incidence reflectivity.
float mx39_ior_to_f0(float ior)
{
    return mx_square((ior - 1.0) / (ior + 1.0));
}

// Convert normal-incidence reflectivity to real-valued index of refraction.
float mx39_f0_to_ior(float F0)
{
    float sqrtF0 = sqrt(clamp(F0, 0.01, 0.99));
    return (1.0 + sqrtF0) / (1.0 - sqrtF0);
}
vec3 mx39_f0_to_ior(vec3 F0)
{
    vec3 sqrtF0 = sqrt(clamp(F0, 0.01, 0.99));
    return (vec3(1.0) + sqrtF0) / (vec3(1.0) - sqrtF0);
}

// https://renderwonk.com/publications/wp-generalization-adobe/gen-adobe.pdf
vec3 mx39_fresnel_hoffman_schlick(float cosTheta, Mx39FresnelData fd)
{
    const float COS_THETA_MAX = 1.0 / 7.0;
    const float COS_THETA_FACTOR = 1.0 / (COS_THETA_MAX * pow(1.0 - COS_THETA_MAX, 6.0));

    float x = clamp(cosTheta, 0.0, 1.0);
    vec3 a = mix(fd.F0, fd.F90, pow(1.0 - COS_THETA_MAX, fd.exponent)) * (vec3(1.0) - fd.F82) * COS_THETA_FACTOR;
    return mix(fd.F0, fd.F90, pow(1.0 - x, fd.exponent)) - a * x * mx39_pow6(1.0 - x);
}

// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
float mx39_fresnel_dielectric(float cosTheta, float ior)
{
    float c = cosTheta;
    float g2 = ior*ior + c*c - 1.0;
    if (g2 < 0.0)
    {
        // Total internal reflection
        return 1.0;
    }

    float g = sqrt(g2);
    return 0.5 * mx_square((g - c) / (g + c)) *
                (1.0 + mx_square(((g + c) * c - 1.0) / ((g - c) * c + 1.0)));
}

// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
vec2 mx39_fresnel_dielectric_polarized(float cosTheta, float ior)
{
    float cosTheta2 = mx_square(clamp(cosTheta, 0.0, 1.0));
    float sinTheta2 = 1.0 - cosTheta2;

    float t0 = max(ior * ior - sinTheta2, 0.0);
    float t1 = t0 + cosTheta2;
    float t2 = 2.0 * sqrt(t0) * cosTheta;
    float Rs = (t1 - t2) / (t1 + t2);

    float t3 = cosTheta2 * t0 + sinTheta2 * sinTheta2;
    float t4 = t2 * sinTheta2;
    float Rp = Rs * (t3 - t4) / (t3 + t4);

    return vec2(Rp, Rs);
}

// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
void mx39_fresnel_conductor_polarized(float cosTheta, vec3 n, vec3 k, out vec3 Rp, out vec3 Rs)
{
    float cosTheta2 = mx_square(clamp(cosTheta, 0.0, 1.0));
    float sinTheta2 = 1.0 - cosTheta2;
    vec3 n2 = n * n;
    vec3 k2 = k * k;

    vec3 t0 = n2 - k2 - vec3(sinTheta2);
    vec3 a2plusb2 = sqrt(t0 * t0 + 4.0 * n2 * k2);
    vec3 t1 = a2plusb2 + vec3(cosTheta2);
    vec3 a = sqrt(max(0.5 * (a2plusb2 + t0), 0.0));
    vec3 t2 = 2.0 * a * cosTheta;
    Rs = (t1 - t2) / (t1 + t2);

    vec3 t3 = cosTheta2 * a2plusb2 + vec3(sinTheta2 * sinTheta2);
    vec3 t4 = t2 * sinTheta2;
    Rp = Rs * (t3 - t4) / (t3 + t4);
}

vec3 mx39_fresnel_conductor(float cosTheta, vec3 n, vec3 k)
{
    vec3 Rp, Rs;
    mx39_fresnel_conductor_polarized(cosTheta, n, k, Rp, Rs);
    return 0.5 * (Rp  + Rs);
}

// https://belcour.github.io/blog/research/publication/2017/05/01/brdf-thin-film.html
void mx39_fresnel_conductor_phase_polarized(float cosTheta, float eta1, vec3 eta2, vec3 kappa2, out vec3 phiP, out vec3 phiS)
{
    vec3 k2 = kappa2 / eta2;
    vec3 sinThetaSqr = vec3(1.0) - cosTheta * cosTheta;
    vec3 A = eta2*eta2*(vec3(1.0)-k2*k2) - eta1*eta1*sinThetaSqr;
    vec3 B = sqrt(A*A + mx_square(2.0*eta2*eta2*k2));
    vec3 U = sqrt((A+B)/2.0);
    vec3 V = max(vec3(0.0), sqrt((B-A)/2.0));

    phiS = atan(2.0*eta1*V*cosTheta, U*U + V*V - mx_square(eta1*cosTheta));
    phiP = atan(2.0*eta1*eta2*eta2*cosTheta * (2.0*k2*U - (vec3(1.0)-k2*k2) * V),
                mx_square(eta2*eta2*(vec3(1.0)+k2*k2)*cosTheta) - eta1*eta1*(U*U+V*V));
}

// A Practical Extension to Microfacet Theory for the Modeling of Varying Iridescence
// https://belcour.github.io/blog/research/publication/2017/05/01/brdf-thin-film.html
vec3 mx39_fresnel_airy(float cosTheta, Mx39FresnelData fd)
{
    // XYZ to CIE 1931 RGB color space (using neutral E illuminant)
    const mat3 XYZ_TO_RGB = mat3(2.3706743, -0.5138850, 0.0052982, -0.9000405, 1.4253036, -0.0146949, -0.4706338, 0.0885814, 1.0093968);

    // Assume vacuum on the outside
    float eta1 = 1.0;
    float eta2 = max(fd.tf_ior, eta1);
    vec3 eta3 = (fd.model == MX39_FRESNEL_MODEL_SCHLICK) ? mx39_f0_to_ior(fd.F0) : fd.ior;
    vec3 kappa3 = (fd.model == MX39_FRESNEL_MODEL_SCHLICK) ? vec3(0.0) : fd.extinction;
    float cosThetaT = sqrt(1.0 - (1.0 - mx_square(cosTheta)) * mx_square(eta1 / eta2));

    // First interface
    vec2 R12 = mx39_fresnel_dielectric_polarized(cosTheta, eta2 / eta1);
    if (cosThetaT <= 0.0)
    {
        // Total internal reflection
        R12 = vec2(1.0);
    }
    vec2 T121 = vec2(1.0) - R12;

    // Second interface
    vec3 R23p, R23s;
    if (fd.model == MX39_FRESNEL_MODEL_SCHLICK)
    {
        vec3 f = mx39_fresnel_hoffman_schlick(cosThetaT, fd);
        R23p = 0.5 * f;
        R23s = 0.5 * f;
    }
    else
    {
        mx39_fresnel_conductor_polarized(cosThetaT, eta3 / eta2, kappa3 / eta2, R23p, R23s);
    }

    // Phase shift
    float cosB = cos(atan(eta2 / eta1));
    vec2 phi21 = vec2(cosTheta < cosB ? 0.0 : M_PI, M_PI);
    vec3 phi23p, phi23s;
    if (fd.model == MX39_FRESNEL_MODEL_SCHLICK)
    {
        phi23p = vec3((eta3[0] < eta2) ? M_PI : 0.0,
                      (eta3[1] < eta2) ? M_PI : 0.0,
                      (eta3[2] < eta2) ? M_PI : 0.0);
        phi23s = phi23p;
    }
    else
    {
        mx39_fresnel_conductor_phase_polarized(cosThetaT, eta2, eta3, kappa3, phi23p, phi23s);
    }
    vec3 r123p = max(sqrt(R12.x*R23p), 0.0);
    vec3 r123s = max(sqrt(R12.y*R23s), 0.0);

    // Iridescence term
    vec3 I = vec3(0.0);
    vec3 Cm, Sm;

    // Optical path difference
    float distMeters = fd.tf_thickness * 1.0e-9;
    float opd = 2.0 * eta2 * cosThetaT * distMeters;

    // Iridescence term using spectral antialiasing for Parallel polarization

    // Reflectance term for m=0 (DC term amplitude)
    vec3 Rs = (mx_square(T121.x) * R23p) / (vec3(1.0) - R12.x*R23p);
    I += R12.x + Rs;

    // Reflectance term for m>0 (pairs of diracs)
    Cm = Rs - T121.x;
    for (int m=1; m<=2; m++)
    {
        Cm *= r123p;
        Sm  = 2.0 * mx_eval_sensitivity(float(m) * opd, float(m)*(phi23p+vec3(phi21.x)));
        I  += Cm*Sm;
    }

    // Iridescence term using spectral antialiasing for Perpendicular polarization

    // Reflectance term for m=0 (DC term amplitude)
    vec3 Rp = (mx_square(T121.y) * R23s) / (vec3(1.0) - R12.y*R23s);
    I += R12.y + Rp;

    // Reflectance term for m>0 (pairs of diracs)
    Cm = Rp - T121.y;
    for (int m=1; m<=2; m++)
    {
        Cm *= r123s;
        Sm  = 2.0 * mx_eval_sensitivity(float(m) * opd, float(m)*(phi23s+vec3(phi21.y)));
        I  += Cm*Sm;
    }

    // Average parallel and perpendicular polarization
    I *= 0.5;

    // Convert back to RGB reflectance
    I = clamp(XYZ_TO_RGB * I, 0.0, 1.0);

    return I;
}

Mx39FresnelData mx39_init_fresnel_dielectric(float ior, float tf_thickness, float tf_ior)
{
    Mx39FresnelData fd;
    fd.model = MX39_FRESNEL_MODEL_DIELECTRIC;
    fd.airy = tf_thickness > 0.0;
    fd.ior = vec3(ior);
    fd.extinction = vec3(0.0);
    fd.F0 = vec3(0.0);
    fd.F82 = vec3(0.0);
    fd.F90 = vec3(0.0);
    fd.exponent = 0.0;
    fd.tf_thickness = tf_thickness;
    fd.tf_ior = tf_ior;
    fd.refraction = false;
    return fd;
}

Mx39FresnelData mx39_init_fresnel_conductor(vec3 ior, vec3 extinction, float tf_thickness, float tf_ior)
{
    Mx39FresnelData fd;
    fd.model = MX39_FRESNEL_MODEL_CONDUCTOR;
    fd.airy = tf_thickness > 0.0;
    fd.ior = ior;
    fd.extinction = extinction;
    fd.F0 = vec3(0.0);
    fd.F82 = vec3(0.0);
    fd.F90 = vec3(0.0);
    fd.exponent = 0.0;
    fd.tf_thickness = tf_thickness;
    fd.tf_ior = tf_ior;
    fd.refraction = false;
    return fd;
}

Mx39FresnelData mx39_init_fresnel_schlick(vec3 F0, vec3 F82, vec3 F90, float exponent, float tf_thickness, float tf_ior)
{
    Mx39FresnelData fd;
    fd.model = MX39_FRESNEL_MODEL_SCHLICK;
    fd.airy = tf_thickness > 0.0;
    fd.ior = vec3(0.0);
    fd.extinction = vec3(0.0);
    fd.F0 = F0;
    fd.F82 = F82;
    fd.F90 = F90;
    fd.exponent = exponent;
    fd.tf_thickness = tf_thickness;
    fd.tf_ior = tf_ior;
    fd.refraction = false;
    return fd;
}

vec3 mx39_compute_fresnel(float cosTheta, Mx39FresnelData fd)
{
    if (fd.airy)
    {
         return mx39_fresnel_airy(cosTheta, fd);
    }
    else if (fd.model == MX39_FRESNEL_MODEL_DIELECTRIC)
    {
        return vec3(mx39_fresnel_dielectric(cosTheta, fd.ior.x));
    }
    else if (fd.model == MX39_FRESNEL_MODEL_CONDUCTOR)
    {
        return mx39_fresnel_conductor(cosTheta, fd.ior, fd.extinction);
    }
    else
    {
        return mx39_fresnel_hoffman_schlick(cosTheta, fd);
    }
}

#ifdef MX39_USING_ENVIRONMENT_NONE
vec3 mx39_environment_radiance(vec3 N, vec3 V, vec3 X, vec2 alpha, int distribution, Mx39FresnelData fd)
{
    return vec3(0.0);
}
#endif

#ifdef MX39_USING_ENVIRONMENT_PREFILTER
vec3 mx39_environment_radiance(vec3 N, vec3 V, vec3 X, vec2 alpha, int distribution, Mx39FresnelData fd)
{
    N = mx_forward_facing_normal(N, V);
    vec3 L = fd.refraction ? mx_refraction_solid_sphere(-V, N, fd.ior.x) : -reflect(V, N);

    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    float avgAlpha = mx_average_alpha(alpha);
    vec3 F = mx39_compute_fresnel(NdotV, fd);
    float G = mx_ggx_smith_G2(NdotV, NdotV, avgAlpha);
    vec3 FG = fd.refraction ? vec3(1.0) - (F * G) : F * G;

    vec3 Li = mx_latlong_map_lookup(L, $envMatrix, mx_latlong_alpha_to_lod(avgAlpha), $envRadiance);
    return Li * FG * $envLightIntensity;
}
#endif

#ifdef MX39_USING_ENVIRONMENT_FIS
vec3 mx39_environment_radiance(vec3 N, vec3 V, vec3 X, vec2 alpha, int distribution, Mx39FresnelData fd)
{
    // Generate tangent frame.
    X = normalize(X - dot(X, N) * N);
    vec3 Y = cross(N, X);
    mat3 tangentToWorld = mat3(X, Y, N);

    // Transform the view vector to tangent space.
    V = vec3(dot(V, X), dot(V, Y), dot(V, N));

    // Compute derived properties.
    float NdotV = clamp(V.z, M_FLOAT_EPS, 1.0);
    float avgAlpha = mx_average_alpha(alpha);
    float G1V = mx_ggx_smith_G1(NdotV, avgAlpha);
    
    // Integrate outgoing radiance using filtered importance sampling.
    // http://cgg.mff.cuni.cz/~jaroslav/papers/2008-egsr-fis/2008-egsr-fis-final-embedded.pdf
    vec3 radiance = vec3(0.0);
    int envRadianceSamples = $envRadianceSamples;
    for (int i = 0; i < envRadianceSamples; i++)
    {
        vec2 Xi = mx_spherical_fibonacci(i, envRadianceSamples);

        // Compute the half vector and incoming light direction.
        vec3 H = mx_ggx_importance_sample_VNDF(Xi, V, alpha);
        vec3 L = fd.refraction ? mx_refraction_solid_sphere(-V, H, fd.ior.x) : -reflect(V, H);
        
        // Compute dot products for this sample.
        float NdotL = clamp(L.z, M_FLOAT_EPS, 1.0);
        float VdotH = clamp(dot(V, H), M_FLOAT_EPS, 1.0);

        // Sample the environment light from the given direction.
        vec3 Lw = tangentToWorld * L;
        float pdf = mx_ggx_NDF(H, alpha) * G1V / (4.0 * NdotV);
        float lod = mx_latlong_compute_lod(Lw, pdf, float($envRadianceMips - 1), envRadianceSamples);
        vec3 sampleColor = mx_latlong_map_lookup(Lw, $envMatrix, lod, $envRadiance);

        // Compute the Fresnel term.
        vec3 F = mx39_compute_fresnel(VdotH, fd);

        // Compute the geometric term.
        float G = mx_ggx_smith_G2(NdotL, NdotV, avgAlpha);

        // Compute the combined FG term, which is inverted for refraction.
        vec3 FG = fd.refraction ? vec3(1.0) - (F * G) : F * G;

        // Add the radiance contribution of this sample.
        // From https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
        //   incidentLight = sampleColor * NdotL
        //   microfacetSpecular = D * F * G / (4 * NdotL * NdotV)
        //   pdf = D * G1V / (4 * NdotV);
        //   radiance = incidentLight * microfacetSpecular / pdf
        radiance += sampleColor * FG;
    }

    // Apply the global component of the geometric term and normalize.
    radiance /= G1V * float(envRadianceSamples);

    // Return the final radiance.
    return radiance * $envLightIntensity;
}
#endif

#ifdef MX39_USING_TRANSMISSION_OPACITY
vec3 mx39_surface_transmission(vec3 N, vec3 V, vec3 X, vec2 alpha, int distribution, Mx39FresnelData fd, vec3 tint)
{
    return tint;
}
#endif

#ifdef MX39_USING_TRANSMISSION_REFRACT
vec3 mx39_surface_transmission(vec3 N, vec3 V, vec3 X, vec2 alpha, int distribution, Mx39FresnelData fd, vec3 tint)
{
    // Approximate the appearance of surface transmission as glossy
    // environment map refraction, ignoring any scene geometry that might
    // be visible through the surface.
    fd.refraction = true;
    if ($refractionTwoSided)
    {
        tint = mx_square(tint);
    }
    return mx39_environment_radiance(N, V, X, alpha, distribution, fd) * tint;
}
#endif
