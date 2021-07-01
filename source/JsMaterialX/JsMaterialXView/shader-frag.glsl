#version 300 es

precision mediump float;

#define M_PI 3.1415926535897932384626433832795
#define M_PI_INV 1.0/3.1415926535897932384626433832795
#define M_GOLDEN_RATIO 1.6180339887498948482045868343656
#define M_FLOAT_EPS 1e-8
#define MAX_LIGHT_SOURCES 1
#define DIRECTIONAL_ALBEDO_METHOD 0
#define MAX_ENV_RADIANCE_SAMPLES 1024

#define BSDF vec3
#define EDF vec3
struct VDF { vec3 absorption; vec3 scattering; };
struct surfaceshader { vec3 color; vec3 transparency; };
struct volumeshader { VDF vdf; EDF edf; };
struct displacementshader { vec3 offset; float scale; };
struct lightshader { vec3 intensity; vec3 direction; };
struct thinfilm { float thickness; float ior; };

// Uniform block: PrivateUniforms
uniform mat4 u_envMatrix;
uniform sampler2D u_envRadiance;
uniform int u_envRadianceMips;
uniform int u_envRadianceSamples;
uniform sampler2D u_envIrradiance;
uniform vec3 u_viewPosition;
uniform int u_numActiveLightSources;

// Uniform block: PublicUniforms
uniform float base;
uniform vec3 base_color;
uniform float diffuse_roughness;
uniform float metalness;
uniform float specular;
uniform vec3 specular_color;
uniform float specular_roughness;
uniform float specular_IOR;
uniform float specular_anisotropy;
uniform float specular_rotation;
uniform float transmission;
uniform vec3 transmission_color;
uniform float transmission_depth;
uniform vec3 transmission_scatter;
uniform float transmission_scatter_anisotropy;
uniform float transmission_dispersion;
uniform float transmission_extra_roughness;
uniform float subsurface;
uniform vec3 subsurface_color;
uniform vec3 subsurface_radius;
uniform float subsurface_scale;
uniform float subsurface_anisotropy;
uniform float sheen;
uniform vec3 sheen_color;
uniform float sheen_roughness;
uniform float coat;
uniform vec3 coat_color;
uniform float coat_roughness;
uniform float coat_anisotropy;
uniform float coat_rotation;
uniform float coat_IOR;
uniform float coat_affect_color;
uniform float coat_affect_roughness;
uniform float thin_film_thickness;
uniform float thin_film_IOR;
uniform float emission;
uniform vec3 emission_color;
uniform vec3 opacity;
uniform bool thin_walled;

struct LightData
{
    int type;
    vec3 direction;
    vec3 color;
    float intensity;
};

uniform LightData u_lightData[MAX_LIGHT_SOURCES];

in vec3 normalWorld;
in vec3 tangentWorld;
in vec3 positionWorld;

// Pixel shader outputs
out vec4 out1;

float mx_square(float x)
{
    return x*x;
}

vec2 mx_square(vec2 x)
{
    return x*x;
}

vec3 mx_square(vec3 x)
{
    return x*x;
}

vec4 mx_square(vec4 x)
{
    return x*x;
}

float mx_pow5(float x)
{
    return mx_square(mx_square(x)) * x;
}

float mx_max_component(vec2 v)
{
    return max(v.x, v.y);
}

float mx_max_component(vec3 v)
{
    return max(max(v.x, v.y), v.z);
}

float mx_max_component(vec4 v)
{
    return max(max(max(v.x, v.y), v.z), v.w);
}

bool mx_is_tiny(float v)
{
    return abs(v) < M_FLOAT_EPS;
}

bool mx_is_tiny(vec3 v)
{
    return all(lessThan(abs(v), vec3(M_FLOAT_EPS)));
}

float mx_mix(float v00, float v01, float v10, float v11,
             float x, float y)
{
   float v0_ = mix(v00, v01, x);
   float v1_ = mix(v10, v11, x);
   return mix(v0_, v1_, y);
}

vec2 mx_latlong_projection(vec3 dir)
{
    float latitude = -asin(dir.y) * M_PI_INV + 0.5;
    float longitude = atan(dir.x, -dir.z) * M_PI_INV * 0.5 + 0.5;
    return vec2(longitude, latitude);
}

vec3 mx_latlong_map_lookup(vec3 dir, mat4 transform, float lod, sampler2D sampler)
{
    vec3 envDir = normalize((transform * vec4(dir,0.0)).xyz);
    vec2 uv = mx_latlong_projection(envDir);
    return textureLod(sampler, uv, lod).rgb;
}

vec3 mx_forward_facing_normal(vec3 N, vec3 V)
{
    if (dot(N, V) < 0.0)
    {
        return -N;
    }
    else
    {
        return N;
    }
}

// Standard Schlick Fresnel
float mx_fresnel_schlick(float cosTheta, float F0)
{
    float x = clamp(1.0 - cosTheta, 0.0, 1.0);
    float x5 = mx_pow5(x);
    return F0 + (1.0 - F0) * x5;
}
vec3 mx_fresnel_schlick(float cosTheta, vec3 F0)
{
    float x = clamp(1.0 - cosTheta, 0.0, 1.0);
    float x5 = mx_pow5(x);
    return F0 + (1.0 - F0) * x5;
}

// Generalized Schlick Fresnel
float mx_fresnel_schlick(float cosTheta, float F0, float F90)
{
    float x = clamp(1.0 - cosTheta, 0.0, 1.0);
    float x5 = mx_pow5(x);
    return mix(F0, F90, x5);
}
vec3 mx_fresnel_schlick(float cosTheta, vec3 F0, vec3 F90)
{
    float x = clamp(1.0 - cosTheta, 0.0, 1.0);
    float x5 = mx_pow5(x);
    return mix(F0, F90, x5);
}

// Generalized Schlick Fresnel with a variable exponent
float mx_fresnel_schlick(float cosTheta, float F0, float F90, float exponent)
{
    float x = clamp(1.0 - cosTheta, 0.0, 1.0);
    return mix(F0, F90, pow(x, exponent));
}
vec3 mx_fresnel_schlick(float cosTheta, vec3 F0, vec3 F90, float exponent)
{
    float x = clamp(1.0 - cosTheta, 0.0, 1.0);
    return mix(F0, F90, pow(x, exponent));
}

// Compute the average of an anisotropic roughness pair
float mx_average_roughness(vec2 roughness)
{
    return sqrt(roughness.x * roughness.y);
}

// https://www.graphics.rwth-aachen.de/publication/2/jgt.pdf
float mx_golden_ratio_sequence(int i)
{
    return fract((float(i) + 1.0) * M_GOLDEN_RATIO);
}

// https://people.irisa.fr/Ricardo.Marques/articles/2013/SF_CGF.pdf
vec2 mx_spherical_fibonacci(int i, int numSamples)
{
    return vec2((float(i) + 0.5) / float(numSamples), mx_golden_ratio_sequence(i));
}

// https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
// Appendix B.2 Equation 13
float mx_ggx_NDF(vec3 X, vec3 Y, vec3 H, float NdotH, float alphaX, float alphaY)
{
    float XdotH = dot(X, H);
    float YdotH = dot(Y, H);
    float denom = mx_square(XdotH / alphaX) + mx_square(YdotH / alphaY) + mx_square(NdotH);
    return 1.0 / (M_PI * alphaX * alphaY * mx_square(denom));
}

// https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
// Appendix B.1 Equation 3
float mx_ggx_PDF(vec3 X, vec3 Y, vec3 H, float NdotH, float LdotH, float alphaX, float alphaY)
{
    return mx_ggx_NDF(X, Y, H, NdotH, alphaX, alphaY) * NdotH / (4.0 * LdotH);
}

// https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
// Appendix B.2 Equation 15
vec3 mx_ggx_importance_sample_NDF(vec2 Xi, vec3 X, vec3 Y, vec3 N, float alphaX, float alphaY)
{
    float phi = 2.0 * M_PI * Xi.x;
    float tanTheta = sqrt(Xi.y / (1.0 - Xi.y));
    vec3 H = X * (tanTheta * alphaX * cos(phi)) +
             Y * (tanTheta * alphaY * sin(phi)) +
             N;
    return normalize(H);
}

// http://jcgt.org/published/0003/02/03/paper.pdf
// Equations 72 and 99
float mx_ggx_smith_G(float NdotL, float NdotV, float alpha)
{
    float alpha2 = mx_square(alpha);
    float lambdaL = sqrt(alpha2 + (1.0 - alpha2) * mx_square(NdotL));
    float lambdaV = sqrt(alpha2 + (1.0 - alpha2) * mx_square(NdotV));
    return 2.0 / (lambdaL / NdotL + lambdaV / NdotV);
}

// https://www.unrealengine.com/blog/physically-based-shading-on-mobile
vec3 mx_ggx_directional_albedo_curve_fit(float NdotV, float roughness, vec3 F0, vec3 F90)
{
    const vec4 c0 = vec4(-1, -0.0275, -0.572, 0.022);
    const vec4 c1 = vec4( 1,  0.0425,  1.04, -0.04 );
    vec4 r = roughness * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28 * NdotV)) * r.x + r.y;
    vec2 AB = vec2(-1.04, 1.04) * a004 + r.zw;
    return F0 * AB.x + F90 * AB.y;
}

vec3 mx_ggx_directional_albedo_table_lookup(float NdotV, float roughness, vec3 F0, vec3 F90)
{
#if DIRECTIONAL_ALBEDO_METHOD == 1
    vec2 res = textureSize(u_albedoTable, 0);
    if (res.x > 1)
    {
        vec2 AB = texture(u_albedoTable, vec2(NdotV, roughness)).rg;
        return F0 * AB.x + F90 * AB.y;
    }
#endif
    return vec3(0.0);
}

// https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
vec3 mx_ggx_directional_albedo_importance_sample(float NdotV, float roughness, vec3 F0, vec3 F90)
{
    NdotV = clamp(NdotV, M_FLOAT_EPS, 1.0);
    vec3 V = vec3(sqrt(1.0f - mx_square(NdotV)), 0, NdotV);

    vec2 AB = vec2(0.0);
    const int SAMPLE_COUNT = 64;
    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
        vec2 Xi = mx_spherical_fibonacci(i, SAMPLE_COUNT);

        // Compute the half vector and incoming light direction.
        vec3 H = mx_ggx_importance_sample_NDF(Xi, vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1), roughness, roughness);
        vec3 L = -reflect(V, H);
        
        // Compute dot products for this sample.
        float NdotL = clamp(L.z, M_FLOAT_EPS, 1.0);
        float NdotH = clamp(H.z, M_FLOAT_EPS, 1.0);
        float VdotH = clamp(dot(V, H), M_FLOAT_EPS, 1.0);

        // Compute the Fresnel term.
        float Fc = mx_fresnel_schlick(VdotH, 0.0, 1.0);

        // Compute the geometric visibility term.
        float Gvis = mx_ggx_smith_G(NdotL, NdotV, roughness) * VdotH / (NdotH * NdotV);
        
        // Add the contribution of this sample.
        AB += vec2(Gvis * (1.0 - Fc), Gvis * Fc);
    }

    // Normalize integrated terms.
    AB /= float(SAMPLE_COUNT);

    // Return the final directional albedo.
    return F0 * AB.x + F90 * AB.y;
}

vec3 mx_ggx_directional_albedo(float NdotV, float roughness, vec3 F0, vec3 F90)
{
#if DIRECTIONAL_ALBEDO_METHOD == 0
    return mx_ggx_directional_albedo_curve_fit(NdotV, roughness, F0, F90);
#elif DIRECTIONAL_ALBEDO_METHOD == 1
    return mx_ggx_directional_albedo_table_lookup(NdotV, roughness, F0, F90);
#else
    return mx_ggx_directional_albedo_importance_sample(NdotV, roughness, F0, F90);
#endif
}

float mx_ggx_directional_albedo(float NdotV, float roughness, float F0, float F90)
{
    return mx_ggx_directional_albedo(NdotV, roughness, vec3(F0), vec3(F90)).x;
}

// https://blog.selfshadow.com/publications/turquin/ms_comp_final.pdf
// Equations 14 and 16
vec3 mx_ggx_energy_compensation(float NdotV, float roughness, vec3 Fss)
{
    float Ess = mx_ggx_directional_albedo(NdotV, roughness, 1.0, 1.0);
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

int numRadianceSamples()
{
    return min(u_envRadianceSamples, MAX_ENV_RADIANCE_SAMPLES);
}

// https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html
// Section 20.4 Equation 13
float mx_latlong_compute_lod(vec3 dir, float pdf, float maxMipLevel, int envSamples)
{
    const float MIP_LEVEL_OFFSET = 1.5;
    float effectiveMaxMipLevel = maxMipLevel - MIP_LEVEL_OFFSET;
    float distortion = sqrt(1.0 - mx_square(dir.y));
    return max(effectiveMaxMipLevel - 0.5 * log2(float(envSamples) * pdf * distortion), 0.0);
}

vec3 mx_environment_radiance(vec3 N, vec3 V, vec3 X, vec2 roughness, int distribution, FresnelData fd)
{
    vec3 Y = normalize(cross(N, X));
    X = cross(Y, N);

    // Compute shared dot products.
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);
    
    // Integrate outgoing radiance using filtered importance sampling.
    // http://cgg.mff.cuni.cz/~jaroslav/papers/2008-egsr-fis/2008-egsr-fis-final-embedded.pdf
    vec3 radiance = vec3(0.0);
    int envRadianceSamples = numRadianceSamples();	
    for (int i = 0; i < envRadianceSamples; i++)
    {
        vec2 Xi = mx_spherical_fibonacci(i, envRadianceSamples);

        // Compute the half vector and incoming light direction.
        vec3 H = mx_ggx_importance_sample_NDF(Xi, X, Y, N, roughness.x, roughness.y);
        vec3 L = -reflect(V, H);
        
        // Compute dot products for this sample.
        float NdotH = clamp(dot(N, H), M_FLOAT_EPS, 1.0);
        float NdotL = clamp(dot(N, L), M_FLOAT_EPS, 1.0);
        float VdotH = clamp(dot(V, H), M_FLOAT_EPS, 1.0);
        float LdotH = VdotH;

        // Sample the environment light from the given direction.
        float pdf = mx_ggx_PDF(X, Y, H, NdotH, LdotH, roughness.x, roughness.y);
        float lod = mx_latlong_compute_lod(L, pdf, float(u_envRadianceMips - 1), envRadianceSamples);
        vec3 sampleColor = mx_latlong_map_lookup(L, u_envMatrix, lod, u_envRadiance);

        // Compute the Fresnel term.
        vec3 F = mx_compute_fresnel(VdotH, fd);

        // Compute the geometric term.
        float G = mx_ggx_smith_G(NdotL, NdotV, mx_average_roughness(roughness));
        
        // Add the radiance contribution of this sample.
        // From https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
        //   incidentLight = sampleColor * NdotL
        //   microfacetSpecular = D * F * G / (4 * NdotL * NdotV)
        //   pdf = D * NdotH / (4 * VdotH)
        //   radiance = incidentLight * microfacetSpecular / pdf
        radiance += sampleColor * F * G * VdotH / (NdotV * NdotH);
    }

    // Normalize and return the final radiance.
    radiance /= float(envRadianceSamples);
    return radiance;
}

vec3 mx_environment_irradiance(vec3 N)
{
    return mx_latlong_map_lookup(N, u_envMatrix, 0.0, u_envIrradiance);
}

void mx_directional_light(LightData light, vec3 position, out lightshader result)
{
    result.direction = -light.direction;
    result.intensity = light.color * light.intensity;
}

int numActiveLightSources()
{
    return min(u_numActiveLightSources, MAX_LIGHT_SOURCES) ;
}

void sampleLightSource(LightData light, vec3 position, out lightshader result)
{
    result.intensity = vec3(0.0);
    result.direction = vec3(0.0);
    if (light.type == 1)
    {
        mx_directional_light(light, position, result);
    }
}

void mx_roughness_anisotropy(float roughness, float anisotropy, out vec2 result)
{
    float roughness_sqr = clamp(roughness*roughness, M_FLOAT_EPS, 1.0);
    if (anisotropy > 0.0)
    {
        float aspect = sqrt(1.0 - clamp(anisotropy, 0.0, 0.98));
        result.x = min(roughness_sqr / aspect, 1.0);
        result.y = roughness_sqr * aspect;
    }
    else
    {
        result.x = roughness_sqr;
        result.y = roughness_sqr;
    }
}

void mx_luminance_color3(vec3 _in, vec3 lumacoeffs, out vec3 result)
{
    result = vec3(dot(_in, lumacoeffs));
}


// http://www.aconty.com/pdf/s2017_pbs_imageworks_sheen.pdf
// Equation 2
float mx_imageworks_sheen_NDF(float cosTheta, float roughness)
{
    float invRoughness = 1.0 / max(roughness, 0.0001);
    float cos2 = cosTheta * cosTheta;
    float sin2 = 1.0 - cos2;
    return (2.0 + invRoughness) * pow(sin2, invRoughness * 0.5) / (2.0 * M_PI);
}

// LUT for sheen directional albedo. 
// A 2D table parameterized with 'cosTheta' (cosine of angle to normal) on x-axis and 'roughness' on y-axis.
#define SHEEN_ALBEDO_TABLE_SIZE 16
const float u_sheenAlbedo[SHEEN_ALBEDO_TABLE_SIZE*SHEEN_ALBEDO_TABLE_SIZE] = float[](
    1.6177, 0.978927, 0.618938, 0.391714, 0.245177, 0.150234, 0.0893475, 0.0511377, 0.0280191, 0.0144204, 0.00687674, 0.00295935, 0.00111049, 0.000336768, 7.07119e-05, 6.22646e-06,
    1.1084, 0.813928, 0.621389, 0.479304, 0.370299, 0.284835, 0.21724, 0.163558, 0.121254, 0.0878921, 0.0619052, 0.0419894, 0.0270556, 0.0161443, 0.00848212, 0.00342323,
    0.930468, 0.725652, 0.586532, 0.479542, 0.393596, 0.322736, 0.26353, 0.213565, 0.171456, 0.135718, 0.105481, 0.0800472, 0.0588117, 0.0412172, 0.0268329, 0.0152799,
    0.833791, 0.671201, 0.558957, 0.471006, 0.398823, 0.337883, 0.285615, 0.240206, 0.200696, 0.16597, 0.135422, 0.10859, 0.0850611, 0.0644477, 0.0464763, 0.0308878,
    0.771692, 0.633819, 0.537877, 0.461939, 0.398865, 0.344892, 0.297895, 0.256371, 0.219562, 0.186548, 0.156842, 0.130095, 0.10598, 0.0841919, 0.0645311, 0.04679,
    0.727979, 0.606373, 0.52141, 0.453769, 0.397174, 0.348337, 0.305403, 0.267056, 0.232655, 0.201398, 0.17286, 0.146756, 0.122808, 0.100751, 0.0804254, 0.0616485,
    0.695353, 0.585281, 0.508227, 0.44667, 0.394925, 0.350027, 0.310302, 0.274561, 0.242236, 0.212604, 0.185281, 0.16002, 0.13657, 0.114693, 0.0942543, 0.0750799,
    0.669981, 0.568519, 0.497442, 0.440542, 0.392567, 0.350786, 0.313656, 0.280075, 0.249533, 0.221359, 0.195196, 0.170824, 0.148012, 0.126537, 0.106279, 0.0870713,
    0.649644, 0.554855, 0.488453, 0.435237, 0.390279, 0.351028, 0.316036, 0.284274, 0.255266, 0.228387, 0.203297, 0.179796, 0.157665, 0.136695, 0.116774, 0.0977403,
    0.632951, 0.543489, 0.480849, 0.430619, 0.388132, 0.350974, 0.317777, 0.287562, 0.259885, 0.234153, 0.210041, 0.187365, 0.165914, 0.145488, 0.125983, 0.10724,
    0.61899, 0.533877, 0.47433, 0.426573, 0.386145, 0.35075, 0.319078, 0.290197, 0.263681, 0.238971, 0.215746, 0.193838, 0.173043, 0.153167, 0.134113, 0.115722,
    0.607131, 0.52564, 0.468678, 0.423001, 0.38432, 0.35043, 0.320072, 0.292349, 0.266856, 0.243055, 0.220636, 0.199438, 0.179264, 0.159926, 0.141332, 0.123323,
    0.596927, 0.518497, 0.463731, 0.419829, 0.382647, 0.350056, 0.320842, 0.294137, 0.269549, 0.246564, 0.224875, 0.204331, 0.18474, 0.165919, 0.147778, 0.130162,
    0.588052, 0.512241, 0.459365, 0.416996, 0.381114, 0.349657, 0.321448, 0.295641, 0.271862, 0.24961, 0.228584, 0.208643, 0.189596, 0.171266, 0.153566, 0.136341,
    0.580257, 0.506717, 0.455481, 0.41445, 0.379708, 0.34925, 0.321929, 0.296923, 0.273869, 0.252279, 0.231859, 0.212472, 0.193933, 0.176066, 0.158788, 0.141945,
    0.573355, 0.5018, 0.452005, 0.412151, 0.378416, 0.348844, 0.322316, 0.298028, 0.275627, 0.254638, 0.234772, 0.215896, 0.197828, 0.180398, 0.163522, 0.147049
);

float mx_imageworks_sheen_directional_albedo(float cosTheta, float roughness)
{
    float x = cosTheta  * float(SHEEN_ALBEDO_TABLE_SIZE - 1);
    float y = roughness * float(SHEEN_ALBEDO_TABLE_SIZE - 1);
    int ix = int(x);
    int iy = int(y);
    int ix2 = clamp(ix + 1, 0, SHEEN_ALBEDO_TABLE_SIZE - 1);
    int iy2 = clamp(iy + 1, 0, SHEEN_ALBEDO_TABLE_SIZE - 1);
    float fx = x - float(ix);
    float fy = y - float(iy);

    // Bi-linear interpolation of the LUT values
    float v1 = mix(u_sheenAlbedo[iy  * SHEEN_ALBEDO_TABLE_SIZE + ix], u_sheenAlbedo[iy  * SHEEN_ALBEDO_TABLE_SIZE + ix2], fx);
    float v2 = mix(u_sheenAlbedo[iy2 * SHEEN_ALBEDO_TABLE_SIZE + ix], u_sheenAlbedo[iy2 * SHEEN_ALBEDO_TABLE_SIZE + ix2], fx);
    float albedo = mix(v1, v2, fy);

    return clamp(albedo, 0.0, 1.0);
}

void mx_sheen_bsdf_reflection(vec3 L, vec3 V, vec3 P, float occlusion, float weight, vec3 color, float roughness, vec3 N, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    N = mx_forward_facing_normal(N, V);

    vec3 H = normalize(L + V);

    float NdotL = clamp(dot(N, L), M_FLOAT_EPS, 1.0);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);
    float NdotH = clamp(dot(N, H), M_FLOAT_EPS, 1.0);

    float D = mx_imageworks_sheen_NDF(NdotH, roughness);

    // Geometry term is skipped and we use a smoother denominator, as in:
    // https://blog.selfshadow.com/publications/s2013-shading-course/rad/s2013_pbs_rad_notes.pdf
    vec3 fr = D * color / (4.0 * (NdotL + NdotV - NdotL*NdotV));

    float dirAlbedo = mx_imageworks_sheen_directional_albedo(NdotV, roughness);

    // We need to include NdotL from the light integral here
    // as in this case it's not cancelled out by the BRDF denominator.
    result = fr * NdotL * occlusion * weight        // Top layer reflection
           + base * (1.0 - dirAlbedo * weight);     // Base layer reflection attenuated by top layer
}

void mx_sheen_bsdf_indirect(vec3 V, float weight, vec3 color, float roughness, vec3 N, BSDF base, out vec3 result)
{
    if (weight <= 0.0)
    {
        result = base;
        return;
    }

    N = mx_forward_facing_normal(N, V);

    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    float dirAlbedo = mx_imageworks_sheen_directional_albedo(NdotV, roughness);

    vec3 Li = mx_environment_irradiance(N);
    result = Li * color * dirAlbedo * weight        // Top layer reflection
             + base * (1.0 - dirAlbedo * weight);   // Base layer reflection attenuated by top layer
}

mat4 mx_rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void mx_rotate_vector3(vec3 _in, float amount, vec3 axis, out vec3 result)
{
    float rotationRadians = radians(amount);
    mat4 m = mx_rotationMatrix(axis, rotationRadians);
    result = (m * vec4(_in, 1.0)).xyz;
}

// "Artist Friendly Metallic Fresnel", Ole Gulbrandsen, 2014
// http://jcgt.org/published/0003/04/03/paper.pdf

void mx_complex_to_artistic_ior(vec3 ior, vec3 extinction, out vec3 reflectivity, out vec3 edge_color)
{
    vec3 nm1 = ior - 1.0;
    vec3 np1 = ior + 1.0;
    vec3 k2  = extinction * extinction;
    vec3 r = (nm1*nm1 + k2) / (np1*np1 + k2);
    reflectivity = r;

    vec3 r_sqrt = sqrt(r);
    vec3 n_min = (1.0 - r) / (1.0 + r);
    vec3 n_max = (1.0 + r_sqrt) / (1.0 - r_sqrt);
    edge_color = (n_max - ior) / (n_max - n_min);
}

void mx_artistic_to_complex_ior(vec3 reflectivity, vec3 edge_color, out vec3 ior, out vec3 extinction)
{
    vec3 r = clamp(reflectivity, 0.0, 0.99);
    vec3 r_sqrt = sqrt(r);
    vec3 n_min = (1.0 - r) / (1.0 + r);
    vec3 n_max = (1.0 + r_sqrt) / (1.0 - r_sqrt);
    ior = mix(n_max, n_min, edge_color);

    vec3 np1 = ior + 1.0;
    vec3 nm1 = ior - 1.0;
    vec3 k2 = (np1*np1 * r - nm1*nm1) / (1.0 - r);
    k2 = max(k2, 0.0);
    extinction = sqrt(k2);
}

void mx_artistic_ior(vec3 reflectivity, vec3 edge_color, out vec3 ior, out vec3 extinction)
{
    mx_artistic_to_complex_ior(reflectivity, edge_color, ior, extinction);
}

void mx_uniform_edf(vec3 N, vec3 L, vec3 color, out EDF result)
{
    result = color;
}


void mx_dielectric_bsdf_reflection(vec3 L, vec3 V, vec3 P, float occlusion, float weight, vec3 tint, float ior, vec2 roughness, vec3 N, vec3 X, int distribution, int scatter_mode, BSDF base, thinfilm tf, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    N = mx_forward_facing_normal(N, V);

    vec3 Y = normalize(cross(N, X));
    vec3 H = normalize(L + V);

    float NdotL = clamp(dot(N, L), M_FLOAT_EPS, 1.0);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);
    float NdotH = clamp(dot(N, H), M_FLOAT_EPS, 1.0);
    float VdotH = clamp(dot(V, H), M_FLOAT_EPS, 1.0);

    float avgRoughness = mx_average_roughness(roughness);

    FresnelData fd;
    if (tf.thickness > 0.0) 
        fd = mx_init_fresnel_dielectric_airy(ior, tf.thickness, tf.ior);
    else
        fd = mx_init_fresnel_dielectric(ior);

    vec3  F = mx_compute_fresnel(VdotH, fd);
    float D = mx_ggx_NDF(X, Y, H, NdotH, roughness.x, roughness.y);
    float G = mx_ggx_smith_G(NdotL, NdotV, avgRoughness);

    float F0 = mx_ior_to_f0(ior);
    vec3 comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);
    vec3 dirAlbedo = mx_ggx_directional_albedo(NdotV, avgRoughness, F0, 1.0) * comp;

    // Note: NdotL is cancelled out
    result = D * F * G * comp * tint * occlusion * weight / (4.0 * NdotV) // Top layer reflection
           + base * (1.0 - dirAlbedo * weight);                         // Base layer reflection attenuated by top layer
}

void mx_dielectric_bsdf_transmission(vec3 V, float weight, vec3 tint, float ior, vec2 roughness, vec3 N, vec3 X, int distribution, int scatter_mode, BSDF base, thinfilm tf, out BSDF result)
{
    if (scatter_mode == 1)
    {
        result = tint * weight;
        return;
    }

    if (scatter_mode == 2)
    {
        // No external layering in RT mode,
        // the base is always T in this case.
        base = tint * weight;
    }

    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    N = mx_forward_facing_normal(N, V);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    FresnelData fd;
    if (tf.thickness > 0.0)
        fd = mx_init_fresnel_dielectric_airy(ior, tf.thickness, tf.ior);
    else
        fd = mx_init_fresnel_dielectric(ior);

    vec3 F = mx_compute_fresnel(NdotV, fd);

    float avgRoughness = mx_average_roughness(roughness);
    float F0 = mx_ior_to_f0(ior);
    vec3 comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);
    vec3 dirAlbedo = mx_ggx_directional_albedo(NdotV, avgRoughness, F0, 1.0) * comp;

    result = base * (1.0 - dirAlbedo * weight); // Transmission attenuated by reflection amount
}

void mx_dielectric_bsdf_indirect(vec3 V, float weight, vec3 tint, float ior, vec2 roughness, vec3 N, vec3 X, int distribution, int scatter_mode, BSDF base, thinfilm tf, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    N = mx_forward_facing_normal(N, V);

    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    FresnelData fd;
    if (tf.thickness > 0.0)
        fd = mx_init_fresnel_dielectric_airy(ior, tf.thickness, tf.ior);
    else
        fd = mx_init_fresnel_dielectric(ior);

    vec3 F = mx_compute_fresnel(NdotV, fd);

    float avgRoughness = mx_average_roughness(roughness);
    float F0 = mx_ior_to_f0(ior);
    vec3 comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);
    vec3 dirAlbedo = mx_ggx_directional_albedo(NdotV, avgRoughness, F0, 1.0) * comp;

    vec3 Li = mx_environment_radiance(N, V, X, roughness, distribution, fd);

    result = Li * tint * comp * weight          // Top layer reflection
           + base * (1.0 - dirAlbedo * weight); // Base layer reflection attenuated by top layer
}


void mx_conductor_bsdf_reflection(vec3 L, vec3 V, vec3 P, float occlusion, float weight, vec3 ior_n, vec3 ior_k, vec2 roughness, vec3 N, vec3 X, int distribution, thinfilm tf, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    N = mx_forward_facing_normal(N, V);

    vec3 Y = normalize(cross(N, X));
    vec3 H = normalize(L + V);

    float NdotL = clamp(dot(N, L), M_FLOAT_EPS, 1.0);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);
    float NdotH = clamp(dot(N, H), M_FLOAT_EPS, 1.0);
    float VdotH = clamp(dot(V, H), M_FLOAT_EPS, 1.0);

    FresnelData fd;
    if (tf.thickness > 0.0)
        fd = mx_init_fresnel_conductor_airy(ior_n, ior_k, tf.thickness, tf.ior);
    else
        fd = mx_init_fresnel_conductor(ior_n, ior_k);

    vec3 F = mx_compute_fresnel(VdotH, fd);

    float avgRoughness = mx_average_roughness(roughness);
    float D = mx_ggx_NDF(X, Y, H, NdotH, roughness.x, roughness.y);
    float G = mx_ggx_smith_G(NdotL, NdotV, avgRoughness);

    vec3 comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);

    // Note: NdotL is cancelled out
    result = D * F * G * comp * occlusion * weight / (4.0 * NdotV);
}

void mx_conductor_bsdf_indirect(vec3 V, float weight, vec3 ior_n, vec3 ior_k, vec2 roughness, vec3 N, vec3 X, int distribution, thinfilm tf, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    N = mx_forward_facing_normal(N, V);

    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    FresnelData fd;
    if (tf.thickness > 0.0)
        fd = mx_init_fresnel_conductor_airy(ior_n, ior_k, tf.thickness, tf.ior);
    else
        fd = mx_init_fresnel_conductor(ior_n, ior_k);

    vec3 F = mx_compute_fresnel(NdotV, fd);

    vec3 Li = mx_environment_radiance(N, V, X, roughness, distribution, fd);

    float avgRoughness = mx_average_roughness(roughness);
    vec3 comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);

    result = Li * comp * weight;
}

// We fake diffuse transmission by using diffuse reflection from the opposite side.
// So this BTDF is really a BRDF.
void mx_translucent_bsdf_reflection(vec3 L, vec3 V, vec3 P, float occlusion, float weight, vec3 color, vec3 normal, out BSDF result)
{
    // Invert normal since we're transmitting light from the other side
    float NdotL = dot(L, -normal);
    if (NdotL <= 0.0 || weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    result = color * weight * NdotL * M_PI_INV;
}

void mx_translucent_bsdf_indirect(vec3 V, float weight, vec3 color, vec3 normal, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    // Invert normal since we're transmitting light from the other side
    vec3 Li = mx_environment_irradiance(-normal);
    result = Li * color * weight;
}


// Based on the OSL implementation of Oren-Nayar diffuse, which is in turn
// based on https://mimosa-pudica.net/improved-oren-nayar.html.
float mx_oren_nayar_diffuse(vec3 L, vec3 V, vec3 N, float NdotL, float roughness)
{
    float LdotV = clamp(dot(L, V), M_FLOAT_EPS, 1.0);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);
    float s = LdotV - NdotL * NdotV;
    float stinv = (s > 0.0f) ? s / max(NdotL, NdotV) : 0.0;

    float sigma2 = mx_square(roughness * M_PI);
    float A = 1.0 - 0.5 * (sigma2 / (sigma2 + 0.33));
    float B = 0.45 * sigma2 / (sigma2 + 0.09);

    return A + B * stinv;
}

// https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
// Section 5.3
float mx_burley_diffuse(vec3 L, vec3 V, vec3 N, float NdotL, float roughness)
{
    vec3 H = normalize(L + V);
    float LdotH = clamp(dot(L, H), M_FLOAT_EPS, 1.0);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    float F90 = 0.5 + (2.0 * roughness * mx_square(LdotH));
    float refL = mx_fresnel_schlick(NdotL, 1.0, F90);
    float refV = mx_fresnel_schlick(NdotV, 1.0, F90);
    return refL * refV;
}

// Compute the directional albedo component of Burley diffuse for the given
// view angle and roughness.  Curve fit provided by Stephen Hill.
float mx_burley_diffuse_directional_albedo(float NdotV, float roughness)
{
    float x = NdotV;
    float fit0 = 0.97619 - 0.488095 * mx_pow5(1.0 - x);
    float fit1 = 1.55754 + (-2.02221 + (2.56283 - 1.06244 * x) * x) * x;
    return mix(fit0, fit1, roughness);
}

// Evaluate the Burley diffusion profile for the given distance and diffusion shape.
// Based on https://graphics.pixar.com/library/ApproxBSSRDF/
vec3 mx_burley_diffusion_profile(float dist, vec3 shape)
{
    vec3 num1 = exp(-shape * dist);
    vec3 num2 = exp(-shape * dist / 3.0);
    float denom = max(dist, M_FLOAT_EPS);
    return (num1 + num2) / denom;
}

// Integrate the Burley diffusion profile over a sphere of the given radius.
// Inspired by Eric Penner's presentation in http://advances.realtimerendering.com/s2011/
vec3 mx_integrate_burley_diffusion(vec3 N, vec3 L, float radius, vec3 mfp)
{
    float theta = acos(dot(N, L));

    // Estimate the Burley diffusion shape from mean free path.
    vec3 shape = vec3(1.0) / max(mfp, 0.1);

    // Integrate the profile over the sphere.
    vec3 sumD = vec3(0.0);
    vec3 sumR = vec3(0.0);
    const int SAMPLE_COUNT = 32;
    const float SAMPLE_WIDTH = (2.0 * M_PI) / float(SAMPLE_COUNT);
    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
        float x = -M_PI + (float(i) + 0.5) * SAMPLE_WIDTH;
        float dist = radius * abs(2.0 * sin(x * 0.5));
        vec3 R = mx_burley_diffusion_profile(dist, shape);
        sumD += R * max(cos(theta + x), 0.0);
        sumR += R;
    }

    return sumD / sumR;
}

vec3 mx_subsurface_scattering_approx(vec3 N, vec3 L, vec3 P, vec3 albedo, vec3 mfp)
{
    float curvature = length(fwidth(N)) / length(fwidth(P));
    float radius = 1.0 / max(curvature, 0.01);
    return albedo * mx_integrate_burley_diffusion(N, L, radius, mfp) / vec3(M_PI);
}

void mx_subsurface_bsdf_reflection(vec3 L, vec3 V, vec3 P, float occlusion, float weight, vec3 color, vec3 radius, float anisotropy, vec3 normal, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    normal = mx_forward_facing_normal(normal, V);

    vec3 sss = mx_subsurface_scattering_approx(normal, L, P, color, radius);
    float NdotL = clamp(dot(normal, L), M_FLOAT_EPS, 1.0);
    float visibleOcclusion = 1.0 - NdotL * (1.0 - occlusion);
    result = sss * visibleOcclusion * weight;
}

void mx_subsurface_bsdf_indirect(vec3 V, float weight, vec3 color, vec3 radius, float anisotropy, vec3 normal, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    normal = mx_forward_facing_normal(normal, V);

    // For now, we render indirect subsurface as simple indirect diffuse.
    vec3 Li = mx_environment_irradiance(normal);
    result = Li * color * weight;
}


void mx_oren_nayar_diffuse_bsdf_reflection(vec3 L, vec3 V, vec3 P, float occlusion, float weight, vec3 color, float roughness, vec3 normal, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    normal = mx_forward_facing_normal(normal, V);

    float NdotL = clamp(dot(normal, L), M_FLOAT_EPS, 1.0);

    result = color * occlusion * weight * NdotL * M_PI_INV;
    if (roughness > 0.0)
    {
        result *= mx_oren_nayar_diffuse(L, V, normal, NdotL, roughness);
    }
}

void mx_oren_nayar_diffuse_bsdf_indirect(vec3 V, float weight, vec3 color, float roughness, vec3 normal, out vec3 result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    normal = mx_forward_facing_normal(normal, V);

    vec3 Li = mx_environment_irradiance(normal);
    result = Li * color * weight;
}

void mx_mix_bsdf_reflection(vec3 L, vec3 V, vec3 P, float occlusion, BSDF fg, BSDF bg, float w, out BSDF result)
{
    result = mix(bg, fg, clamp(w, 0.0, 1.0));
}

void mx_mix_bsdf_transmission(vec3 V, BSDF fg, BSDF bg, float w, out BSDF result)
{
    result = mix(bg, fg, clamp(w, 0.0, 1.0));
}

void mx_mix_bsdf_indirect(vec3 V, vec3 fg, vec3 bg, float w, out vec3 result)
{
    result = mix(bg, fg, clamp(w, 0.0, 1.0));
}

void mx_multiply_bsdf_color_reflection(vec3 L, vec3 V, vec3 P, float occlusion, BSDF in1, vec3 in2, out BSDF result)
{
    result = in1 * clamp(in2, 0.0, 1.0);
}

void mx_multiply_bsdf_color_transmission(vec3 V, BSDF in1, vec3 in2, out BSDF result)
{
    result = in1 * clamp(in2, 0.0, 1.0);
}

void mx_multiply_bsdf_color_indirect(vec3 V, vec3 in1, vec3 in2, out vec3 result)
{
    result = in1 * clamp(in2, 0.0, 1.0);
}

void IMPL_standard_surface_surfaceshader(float base, vec3 base_color, float diffuse_roughness, float metalness, float specular, vec3 specular_color, float specular_roughness, float specular_IOR, float specular_anisotropy, float specular_rotation, float transmission, vec3 transmission_color, float transmission_depth, vec3 transmission_scatter, float transmission_scatter_anisotropy, float transmission_dispersion, float transmission_extra_roughness, float subsurface, vec3 subsurface_color, vec3 subsurface_radius, float subsurface_scale, float subsurface_anisotropy, float sheen, vec3 sheen_color, float sheen_roughness, float coat, vec3 coat_color, float coat_roughness, float coat_anisotropy, float coat_rotation, float coat_IOR, vec3 coat_normal, float coat_affect_color, float coat_affect_roughness, float thin_film_thickness, float thin_film_IOR, float emission, vec3 emission_color, vec3 opacity, bool thin_walled, vec3 normal1, vec3 tangent1, out surfaceshader out1)
{
    vec3 emission_weight_out = emission_color * emission;
    vec3 metal_reflectivity_out = base_color * base;
    const float coat_tangent_rotate_degree_in2_tmp = 360.000000;
    float coat_tangent_rotate_degree_out = coat_rotation * coat_tangent_rotate_degree_in2_tmp;
    vec2 coat_roughness_vector_out = vec2(0.0);
    mx_roughness_anisotropy(coat_roughness, coat_anisotropy, coat_roughness_vector_out);
    vec3 subsurface_radius_vector_out = vec3(subsurface_radius.x, subsurface_radius.y, subsurface_radius.z);
    vec3 opacity_luminance_out = vec3(0.0);
    mx_luminance_color3(opacity, vec3(0.272229, 0.674082, 0.053689), opacity_luminance_out);
    const vec3 coat_emission_attenuation_bg_tmp = vec3(1.000000, 1.000000, 1.000000);
    vec3 coat_emission_attenuation_out = mix(coat_emission_attenuation_bg_tmp, coat_color, coat);
    vec3 metal_edgecolor_out = specular_color * specular;
    float coat_affect_roughness_multiply1_out = coat_affect_roughness * coat;
    float subsurface_selector_out = float(thin_walled);
    const float tangent_rotate_degree_in2_tmp = 360.000000;
    float tangent_rotate_degree_out = specular_rotation * tangent_rotate_degree_in2_tmp;
    const float coat_clamped_low_tmp = 0.000000;
    const float coat_clamped_high_tmp = 1.000000;
    float coat_clamped_out = clamp(coat, coat_clamped_low_tmp, coat_clamped_high_tmp);
    const vec3 coat_attenuation_bg_tmp = vec3(1.000000, 1.000000, 1.000000);
    vec3 coat_attenuation_out = mix(coat_attenuation_bg_tmp, coat_color, coat);
    vec3 coat_tangent_rotate_out = vec3(0.0);
    mx_rotate_vector3(tangent1, coat_tangent_rotate_degree_out, coat_normal, coat_tangent_rotate_out);
    vec3 subsurface_radius_scaled_out = subsurface_radius_vector_out * subsurface_scale;
    vec3 emission_weight_attenuated_out = emission_weight_out * coat_emission_attenuation_out;
    vec3 artistic_ior_ior = vec3(0.0);
    vec3 artistic_ior_extinction = vec3(0.0);
    mx_artistic_ior(metal_reflectivity_out, metal_edgecolor_out, artistic_ior_ior, artistic_ior_extinction);
    float coat_affect_roughness_multiply2_out = coat_affect_roughness_multiply1_out * coat_roughness;
    vec3 tangent_rotate_out = vec3(0.0);
    mx_rotate_vector3(tangent1, tangent_rotate_degree_out, normal1, tangent_rotate_out);
    float coat_gamma_multiply_out = coat_clamped_out * coat_affect_color;
    vec3 coat_tangent_rotate_normalize_out = normalize(coat_tangent_rotate_out);
    const float coat_affected_roughness_fg_tmp = 1.000000;
    float coat_affected_roughness_out = mix(specular_roughness, coat_affected_roughness_fg_tmp, coat_affect_roughness_multiply2_out);
    vec3 tangent_rotate_normalize_out = normalize(tangent_rotate_out);
    const float coat_gamma_in2_tmp = 1.000000;
    float coat_gamma_out = coat_gamma_multiply_out + coat_gamma_in2_tmp;
    vec3 coat_tangent_out = vec3(0.0);
    if (coat_anisotropy > 0.000000)
    {
        coat_tangent_out = coat_tangent_rotate_normalize_out;
    }
    else
    {
        coat_tangent_out = tangent1;
    }
    vec2 main_roughness_out = vec2(0.0);
    mx_roughness_anisotropy(coat_affected_roughness_out, specular_anisotropy, main_roughness_out);
    vec3 main_tangent_out = vec3(0.0);
    if (specular_anisotropy > 0.000000)
    {
        main_tangent_out = tangent_rotate_normalize_out;
    }
    else
    {
        main_tangent_out = tangent1;
    }
    vec3 coat_affected_subsurface_color_out = pow(subsurface_color, vec3(coat_gamma_out));
    vec3 coat_affected_diffuse_color_out = pow(base_color, vec3(coat_gamma_out));

    surfaceshader shader_constructor_out = surfaceshader(vec3(0.0),vec3(0.0));
    {
        // Shadow occlusion
        float occlusion = 1.0;

        vec3 N = normalize(normalWorld);
        vec3 V = normalize(u_viewPosition - positionWorld);
        vec3 P = positionWorld;
        // Light loop
        int numLights = numActiveLightSources();
        lightshader lightShader;
        for (int activeLightIndex = 0; activeLightIndex < numLights; ++activeLightIndex)
        {
            sampleLightSource(u_lightData[activeLightIndex], positionWorld, lightShader);
            vec3 L = lightShader.direction;

            // Calculate the BSDF response for this light source
            thinfilm thin_film_bsdf_out;
            thin_film_bsdf_out.thickness = thin_film_thickness;
            thin_film_bsdf_out.ior = thin_film_IOR;
            BSDF metal_bsdf_out = BSDF(0.0);
            mx_conductor_bsdf_reflection(L, V, P, occlusion, 1.000000, artistic_ior_ior, artistic_ior_extinction, main_roughness_out, normal1, main_tangent_out, 0, thinfilm(0.0,1.5), metal_bsdf_out);
            BSDF transmission_bsdf_out = BSDF(0.0);
            BSDF translucent_bsdf_out = BSDF(0.0);
            mx_translucent_bsdf_reflection(L, V, P, occlusion, 1.000000, coat_affected_subsurface_color_out, normal1, translucent_bsdf_out);
            BSDF subsurface_bsdf_out = BSDF(0.0);
            mx_subsurface_bsdf_reflection(L, V, P, occlusion, 1.000000, coat_affected_subsurface_color_out, subsurface_radius_scaled_out, subsurface_anisotropy, normal1, subsurface_bsdf_out);
            BSDF diffuse_bsdf_out = BSDF(0.0);
            mx_oren_nayar_diffuse_bsdf_reflection(L, V, P, occlusion, base, coat_affected_diffuse_color_out, diffuse_roughness, normal1, diffuse_bsdf_out);
            BSDF selected_subsurface_bsdf_out = BSDF(0.0);
            mx_mix_bsdf_reflection(L, V, P, occlusion, translucent_bsdf_out, subsurface_bsdf_out, subsurface_selector_out, selected_subsurface_bsdf_out);
            BSDF subsurface_mix_out = BSDF(0.0);
            mx_mix_bsdf_reflection(L, V, P, occlusion, selected_subsurface_bsdf_out, diffuse_bsdf_out, subsurface, subsurface_mix_out);
            BSDF sheen_layer_out = BSDF(0.0);
            mx_sheen_bsdf_reflection(L, V, P, occlusion, sheen, sheen_color, sheen_roughness, normal1, subsurface_mix_out, sheen_layer_out);
            BSDF transmission_mix_out = BSDF(0.0);
            mx_mix_bsdf_reflection(L, V, P, occlusion, transmission_bsdf_out, sheen_layer_out, transmission, transmission_mix_out);
            BSDF specular_layer_out = BSDF(0.0);
            mx_dielectric_bsdf_reflection(L, V, P, occlusion, specular, specular_color, specular_IOR, main_roughness_out, normal1, main_tangent_out, 0, 0, transmission_mix_out, thin_film_bsdf_out, specular_layer_out);
            BSDF metalness_mix_out = BSDF(0.0);
            mx_mix_bsdf_reflection(L, V, P, occlusion, metal_bsdf_out, specular_layer_out, metalness, metalness_mix_out);
            BSDF metalness_mix_attenuated_out = BSDF(0.0);
            mx_multiply_bsdf_color_reflection(L, V, P, occlusion, metalness_mix_out, coat_attenuation_out, metalness_mix_attenuated_out);
            BSDF coat_layer_out = BSDF(0.0);
            mx_dielectric_bsdf_reflection(L, V, P, occlusion, coat, vec3(1.000000, 1.000000, 1.000000), coat_IOR, coat_roughness_vector_out, coat_normal, coat_tangent_out, 0, 0, metalness_mix_attenuated_out, thinfilm(0.0,1.5), coat_layer_out);

            // Accumulate the light's contribution
            shader_constructor_out.color += lightShader.intensity * coat_layer_out;
        }

        // Ambient occlusion
        occlusion = 1.0;

        // Add surface emission
        {
            EDF emission_edf_out = EDF(0.0);
            mx_uniform_edf(N, V, emission_weight_attenuated_out, emission_edf_out);
            shader_constructor_out.color += emission_edf_out;
        }

        // Add indirect contribution
        {
            thinfilm thin_film_bsdf_out;
            thin_film_bsdf_out.thickness = thin_film_thickness;
            thin_film_bsdf_out.ior = thin_film_IOR;
            BSDF metal_bsdf_out = BSDF(0.0);
            mx_conductor_bsdf_indirect(V, 1.000000, artistic_ior_ior, artistic_ior_extinction, main_roughness_out, normal1, main_tangent_out, 0, thinfilm(0.0,1.5), metal_bsdf_out);
            BSDF transmission_bsdf_out = BSDF(0.0);
            BSDF translucent_bsdf_out = BSDF(0.0);
            mx_translucent_bsdf_indirect(V, 1.000000, coat_affected_subsurface_color_out, normal1, translucent_bsdf_out);
            BSDF subsurface_bsdf_out = BSDF(0.0);
            mx_subsurface_bsdf_indirect(V, 1.000000, coat_affected_subsurface_color_out, subsurface_radius_scaled_out, subsurface_anisotropy, normal1, subsurface_bsdf_out);
            BSDF diffuse_bsdf_out = BSDF(0.0);
            mx_oren_nayar_diffuse_bsdf_indirect(V, base, coat_affected_diffuse_color_out, diffuse_roughness, normal1, diffuse_bsdf_out);
            BSDF selected_subsurface_bsdf_out = BSDF(0.0);
            mx_mix_bsdf_indirect(V, translucent_bsdf_out, subsurface_bsdf_out, subsurface_selector_out, selected_subsurface_bsdf_out);
            BSDF subsurface_mix_out = BSDF(0.0);
            mx_mix_bsdf_indirect(V, selected_subsurface_bsdf_out, diffuse_bsdf_out, subsurface, subsurface_mix_out);
            BSDF sheen_layer_out = BSDF(0.0);
            mx_sheen_bsdf_indirect(V, sheen, sheen_color, sheen_roughness, normal1, subsurface_mix_out, sheen_layer_out);
            BSDF transmission_mix_out = BSDF(0.0);
            mx_mix_bsdf_indirect(V, transmission_bsdf_out, sheen_layer_out, transmission, transmission_mix_out);
            BSDF specular_layer_out = BSDF(0.0);
            mx_dielectric_bsdf_indirect(V, specular, specular_color, specular_IOR, main_roughness_out, normal1, main_tangent_out, 0, 0, transmission_mix_out, thin_film_bsdf_out, specular_layer_out);
            BSDF metalness_mix_out = BSDF(0.0);
            mx_mix_bsdf_indirect(V, metal_bsdf_out, specular_layer_out, metalness, metalness_mix_out);
            BSDF metalness_mix_attenuated_out = BSDF(0.0);
            mx_multiply_bsdf_color_indirect(V, metalness_mix_out, coat_attenuation_out, metalness_mix_attenuated_out);
            BSDF coat_layer_out = BSDF(0.0);
            mx_dielectric_bsdf_indirect(V, coat, vec3(1.000000, 1.000000, 1.000000), coat_IOR, coat_roughness_vector_out, coat_normal, coat_tangent_out, 0, 0, metalness_mix_attenuated_out, thinfilm(0.0,1.5), coat_layer_out);

            shader_constructor_out.color += occlusion * coat_layer_out;
        }

        shader_constructor_out.transparency = vec3(0.0);
    }

    out1 = shader_constructor_out;
}

void main()
{
    vec3 geomprop_Nworld_out = normalize(normalWorld);
    vec3 geomprop_Tworld_out = normalize(tangentWorld);

    surfaceshader SR_default_out = surfaceshader(vec3(0.0),vec3(0.0));
    IMPL_standard_surface_surfaceshader(base, base_color, diffuse_roughness, metalness, specular, specular_color, specular_roughness, specular_IOR, specular_anisotropy, specular_rotation, transmission, transmission_color, transmission_depth, transmission_scatter, transmission_scatter_anisotropy, transmission_dispersion, transmission_extra_roughness, subsurface, subsurface_color, subsurface_radius, subsurface_scale, subsurface_anisotropy, sheen, sheen_color, sheen_roughness, coat, coat_color, coat_roughness, coat_anisotropy, coat_rotation, coat_IOR, geomprop_Nworld_out, coat_affect_color, coat_affect_roughness, thin_film_thickness, thin_film_IOR, emission, emission_color, opacity, thin_walled, geomprop_Nworld_out, geomprop_Tworld_out, SR_default_out);
    out1 = vec4(SR_default_out.color, 1.0);
}
