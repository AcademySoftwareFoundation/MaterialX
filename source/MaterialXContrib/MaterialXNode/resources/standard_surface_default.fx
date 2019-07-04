// ===========================================================================
// Copyright 2017 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//**************************************************************************/ 
// Copyright 2015 Autodesk, Inc. 
// All rights reserved. 
// 
// This computer source code and related instructions and comments are the 
// unpublished confidential and proprietary information of Autodesk, Inc. 
// and are protected under Federal copyright and state trade secret law. 
// They may not be disclosed to, copied or used by any third party without 
// the prior written consent of Autodesk, Inc. 
//**************************************************************************/ 

//
// Compiled Fragment: reqdVtxFormat
//



////////////////////// 
// reqdVtxFormat_VS


attribute vertexInS
{
    // vertexInS
vec3 Pm : POSITION; 
vec3 tangentWorld : TEXCOORD0; 
vec3 normalWorld : TEXCOORD1; 
vec3 positionWorld : TEXCOORD2; 
}

//  Declarations 

attribute vertOutS
{
    // vertOutS
vec3 positionWorld : TEXCOORD0; 
vec3 normalWorld : TEXCOORD1; 
vec3 tangentWorld : TEXCOORD2; 
vec3 Pw : TEXCOORD3; 
}


//  Globals 
uniform mat4 World : world; 
uniform float DepthPriority : depthpriority; 
uniform mat4 WorldViewProj : worldviewprojection; 

attribute fragInS
{
    // fragInS
vec3 positionWorld : TEXCOORD0; 
vec3 normalWorld : TEXCOORD1; 
vec3 tangentWorld : TEXCOORD2; 
vec3 Pw : TEXCOORD3; 
}

attribute fragOutS
{
    vec4 ColorOut;
}

//  Declarations 
struct mayaSurfaceShaderOutput {
	vec3 outColor;
	vec3 outTransparency;
	vec3 outGlowColor;
	vec3 outMatteOpacity;
	vec4 outSurfaceFinal;
};

//  Globals 
uniform mat4 u_envMatrix; 
uniform texture2D u_envIrradiance; 
uniform sampler2D u_envIrradianceSampler= sampler_state
{ 
    Texture = <u_envIrradiance>; 
}; 
uniform texture2D u_envRadiance; 
uniform sampler2D u_envRadianceSampler= sampler_state
{ 
    Texture = <u_envRadiance>; 
}; 
uniform int u_envRadianceMips=1; 
uniform int u_envSamples=16; 
uniform vec3 u_viewPosition : WorldCameraPosition; 
uniform int u_numActiveLightSources=0; 
uniform float base=0.800000; 
uniform vec3 base_color={ 1.000000, 1.000000, 1.000000 }; 
uniform float diffuse_roughness=0.000000; 
uniform float specular=1.000000; 
uniform vec3 specular_color={ 1.000000, 1.000000, 1.000000 }; 
uniform float specular_roughness=0.100000; 
uniform float specular_IOR=1.520000; 
uniform float specular_anisotropy=0.000000; 
uniform float specular_rotation=0.000000; 
uniform float metalness=0.000000; 
uniform float transmission=0.000000; 
uniform vec3 transmission_color={ 1.000000, 1.000000, 1.000000 }; 
uniform float transmission_depth=0.000000; 
uniform vec3 transmission_scatter={ 0.000000, 0.000000, 0.000000 }; 
uniform float transmission_scatter_anisotropy=0.000000; 
uniform float transmission_dispersion=0.000000; 
uniform float transmission_extra_roughness=0.000000; 
uniform float subsurface=0.000000; 
uniform vec3 subsurface_color={ 1.000000, 1.000000, 1.000000 }; 
uniform vec3 subsurface_radius={ 1.000000, 1.000000, 1.000000 }; 
uniform float subsurface_scale=1.000000; 
uniform float subsurface_anisotropy=0.000000; 
uniform float sheen=0.000000; 
uniform vec3 sheen_color={ 1.000000, 1.000000, 1.000000 }; 
uniform float sheen_roughness=0.300000; 
uniform bool thin_walled=0; 
uniform float coat=0.000000; 
uniform vec3 coat_color={ 1.000000, 1.000000, 1.000000 }; 
uniform float coat_roughness=0.100000; 
uniform float coat_anisotropy=0.000000; 
uniform float coat_rotation=0.000000; 
uniform float coat_IOR=1.500000; 
uniform float coat_affect_color=0.000000; 
uniform float coat_affect_roughness=0.000000; 
uniform float thin_film_thickness=0.000000; 
uniform float thin_film_IOR=1.500000; 
uniform float emission=0.000000; 
uniform vec3 emission_color={ 1.000000, 1.000000, 1.000000 }; 
uniform vec3 opacity={ 1.000000, 1.000000, 1.000000 }; 
uniform vec3 surfaceShader_1outTransparency : outTransparency; 
uniform vec3 surfaceShader_1outGlowColor : outGlowColor; 
uniform vec3 surfaceShader_1outMatteOpacity : outMatteOpacity; 
uniform bool mayaAlphaCut : mayaAlphaCut; 
uniform float extraOpacity=1.000000; 
uniform bool fogEnabled=0; 
uniform mat4 ViewProj : viewprojection; 
uniform float fogStart=0.000000; 
uniform float fogEnd=92.000000; 
uniform int fogMode=0; 
uniform float fogDensity=0.100000; 
uniform vec4 fogColor={ 0.500000, 0.500000, 0.500000, 1.000000 }; 
uniform float fogMultiplier=1.000000; 

GLSLShader reqdVtxFormat_VS
{

//  Fragments 
vec3 iNamedUsage0_F3(vec3 val ){ return val; } 

vec3 iNamedUsage1_F3(vec3 val ){ return val; } 

vec3 iNamedUsage2_F3(vec3 val ){ return val; } 

highp vec3 iPw( in vec3 pm, highp mat4 world ) 
{ 
    return ( world * vec4(pm, 1.0) ).xyz; 
} 

highp vec4 iPcPriority(in vec3 pm, in float depthPriority, in highp mat4 worldViewProjectionC) 
{ 
    highp vec4 P = ( worldViewProjectionC * vec4(pm,1.0) ); 
    P.z -= P.w * 2.0 * depthPriority; 
    return P; 
} 



 // Vertex Shader 
void main() 
 { 

     // ShaderBody 
        VS_OUT.positionWorld = iNamedUsage0_F3( positionWorld ); 
    VS_OUT.normalWorld = iNamedUsage1_F3( normalWorld ); 
    VS_OUT.tangentWorld = iNamedUsage2_F3( tangentWorld ); 
    VS_OUT.Pw = iPw( Pm, World ); 
      gl_Position = iPcPriority ( Pm, DepthPriority, WorldViewProj ); 
 
 } 
}


////////////////////// 
// reqdVtxFormat


GLSLShader reqdVtxFormat
{

//  Fragments 
#define M_PI 3.1415926535897932384626433832795
#define M_PI_INV 1.0/3.1415926535897932384626433832795
#define M_GOLDEN_RATIO 1.6180339887498948482045868343656
#define M_FLOAT_EPS 0.000001
#define MAX_LIGHT_SOURCES 1

#define BSDF vec3
#define EDF vec3
struct VDF { vec3 absorption; vec3 scattering; };
struct roughnessinfo { float roughness; float alpha; float alphaX; float alphaY; };
struct surfaceshader { vec3 color; vec3 transparency; };
struct volumeshader { VDF vdf; EDF edf; };
struct displacementshader { vec3 offset; float scale; };
struct lightshader { vec3 intensity; vec3 direction; };

// TODO: Is generated so cannot be added here otherwise it will show up twice.
// Uniform block: PrivateUniforms
//uniform mat4 u_envMatrix = mat4(-1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, -1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000);
//uniform sampler2D u_envIrradianceSampler;
//uniform sampler2D u_envRadianceSampler;
//uniform int u_envRadianceMips = 1;
//uniform int u_envSamples = 16;
//uniform vec3 u_viewPosition = vec3(0.0);
//uniform int u_numActiveLightSources = 0;

// Manually comment out. These should never be output by any generator in the
// first place since it is never used.
//uniform int geomprop_Nworld_space = 2;
//uniform int geomprop_Tworld_space = 2;
//uniform int geomprop_Tworld_index = 0;

struct LightData
{
    int type;
};

uniform LightData u_lightData[MAX_LIGHT_SOURCES];

// Manually commented out. This is only the pixel shader so comment
// out the vertex in and pixel out
// Q: Can make this a input requirement, but how to tell what the name of the
// input structure for the pixel shader. Seems to be PIX_IN for some reason.
//in VertexData
//{
//    vec3 normalWorld;
//    vec3 tangentWorld;
//    vec3 positionWorld;
//} vd;

// Pixel shader outputs
//out vec4 out1;

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

float mx_orennayar(vec3 L, vec3 V, vec3 N, float NdotL, float roughness)
{
    float LdotV = dot(L, V);
    float NdotV = dot(N, V);

    float t = LdotV - NdotL * NdotV;
    t = t > 0.0 ? t / max(NdotL, NdotV) : 0.0;

    float sigma2 = mx_square(roughness * M_PI);
    float A = 1.0 - 0.5 * (sigma2 / (sigma2 + 0.33));
    float B = 0.45f * sigma2 / (sigma2 + 0.09);

    return A + B * t;
}

// https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
// Appendix B.2 Equation 13
float mx_microfacet_ggx_NDF(vec3 X, vec3 Y, vec3 H, float NdotH, float alphaX, float alphaY)
{
    float XdotH = dot(X, H);
    float YdotH = dot(Y, H);
    float denom = mx_square(XdotH / alphaX) + mx_square(YdotH / alphaY) + mx_square(NdotH);
    return 1.0 / (M_PI * alphaX * alphaY * mx_square(denom));
}

// https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
// Appendix B.1 Equation 3
float mx_microfacet_ggx_PDF(vec3 X, vec3 Y, vec3 H, float NdotH, float LdotH, float alphaX, float alphaY)
{
    return mx_microfacet_ggx_NDF(X, Y, H, NdotH, alphaX, alphaY) * NdotH / (4.0 * LdotH);
}

// https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
// Appendix B.2 Equation 15
vec3 mx_microfacet_ggx_IS(vec2 Xi, vec3 X, vec3 Y, vec3 N, float alphaX, float alphaY)
{
    float phi = 2.0 * M_PI * Xi.x;
    float tanTheta = sqrt(Xi.y / (1.0 - Xi.y));
    vec3 H = vec3(X * (tanTheta * alphaX * cos(phi)) +
                  Y * (tanTheta * alphaY * sin(phi)) +
                  N);
    return normalize(H);
}

// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf (Equation 34)
float mx_microfacet_ggx_G1(float cosTheta, float alpha)
{
    float cosTheta2 = cosTheta * cosTheta;
    float tanTheta2 = (1.0 - cosTheta2) / cosTheta2;
    return 2.0 / (1.0 + sqrt(1.0 + alpha * alpha * tanTheta2));
}

// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf (Equation 23)
float mx_microfacet_ggx_smith_G(float NdotL, float NdotV, float alpha)
{
    return mx_microfacet_ggx_G1(NdotL, alpha) * mx_microfacet_ggx_G1(NdotV, alpha);
}

// http://blog.selfshadow.com/publications/s2017-shading-course/imageworks/s2017_pbs_imageworks_sheen.pdf (Equation 2)
float mx_microfacet_sheen_NDF(float cosTheta, float roughness)
{
    // Given roughness is assumed to be clamped to [M_FLOAT_EPS, 1.0]
    float invRoughness = 1.0 / roughness;
    float cos2 = cosTheta * cosTheta;
    float sin2 = 1.0 - cos2;
    return (2.0 + invRoughness) * pow(sin2, invRoughness * 0.5) / (2.0 * M_PI);
}

// LUT for sheen directional albedo. 
// A 2D table parameterized with 'cosTheta' (cosine of angle to normal) on x-axis and 'roughness' on y-axis.
#define SHEEN_ALBEDO_TABLE_SIZE 16
uniform float u_sheenAlbedo[SHEEN_ALBEDO_TABLE_SIZE*SHEEN_ALBEDO_TABLE_SIZE] = float[](
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

float mx_microfacet_sheen_albedo(float cosTheta, float roughness)
{
    float x = cosTheta  * (SHEEN_ALBEDO_TABLE_SIZE - 1);
    float y = roughness * (SHEEN_ALBEDO_TABLE_SIZE - 1);
    int ix = int(x);
    int iy = int(y);
    int ix2 = clamp(ix + 1, 0, SHEEN_ALBEDO_TABLE_SIZE - 1);
    int iy2 = clamp(iy + 1, 0, SHEEN_ALBEDO_TABLE_SIZE - 1);
    float fx = x - ix;
    float fy = y - iy;

    // Bi-linear interpolation of the LUT values
    float v1 = mix(u_sheenAlbedo[iy  * SHEEN_ALBEDO_TABLE_SIZE + ix], u_sheenAlbedo[iy  * SHEEN_ALBEDO_TABLE_SIZE + ix2], fx);
    float v2 = mix(u_sheenAlbedo[iy2 * SHEEN_ALBEDO_TABLE_SIZE + ix], u_sheenAlbedo[iy2 * SHEEN_ALBEDO_TABLE_SIZE + ix2], fx);
    float albedo = mix(v1, v2, fy);

    return clamp(albedo, 0.0, 1.0);
}

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

vec3 mx_fresnel_schlick(float cosTheta, vec3 F0)
{
    if (cosTheta < 0.0)
        return vec3(1.0);
    float x = 1.0 - cosTheta;
    float x2 = x*x;
    float x5 = x2*x2*x;
    return F0 + (1.0 - F0) * x5;
}

float mx_fresnel_schlick(float cosTheta, float ior)
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

float mx_fresnel_schlick_roughness(float cosTheta, float ior, float roughness)
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

// https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
// Section 5.3
float mx_burley_diffuse(vec3 L, vec3 V, vec3 N, float NdotL, float roughness)
{
    vec3 H = normalize(L + V);
    float LdotH = max(dot(L, H), 0.0);
    float NdotV = max(dot(N, V), 0.0);

    float F90 = 0.5 + (2.0 * roughness * mx_square(LdotH));
    float refL = mx_fresnel_schlick(NdotL, 1.0, F90, 5.0);
    float refV = mx_fresnel_schlick(NdotV, 1.0, F90, 5.0);
    return refL * refV * M_PI_INV;
}

// Compute the directional albedo component of Burley diffuse for the given
// view angle and roughness.  Curve fit provided by Stephen Hill.
float mx_burley_directional_albedo(vec3 V, vec3 N, float roughness)
{
    float x = dot(N, V);
    float fit0 = 0.97619 - 0.488095 * mx_pow5(1 - x);
    float fit1 = 1.55754 + (-2.02221 + (2.56283 - 1.06244 * x) * x) * x;
    return mix(fit0, fit1, roughness);
}

vec2 mx_latlong_projection(vec3 dir)
{
    float latitude = -asin(dir.y) * M_PI_INV + 0.5;
    latitude = clamp(latitude, 0.01, 0.99);
    float longitude = atan(dir.x, -dir.z) * M_PI_INV * 0.5 + 0.5;
    return vec2(longitude, latitude);
}

// https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html
// Section 20.4 Equation 13
float mx_latlong_compute_lod(vec3 dir, float pdf, float maxMipLevel, int envSamples)
{
    const float MIP_LEVEL_OFFSET = 1.5;
    float effectiveMaxMipLevel = maxMipLevel - MIP_LEVEL_OFFSET;
    float distortion = sqrt(1.0 - mx_square(dir.y));
    return max(effectiveMaxMipLevel - 0.5 * log2(envSamples * pdf * distortion), 0.0);
}

vec3 mx_latlong_map_lookup(vec3 dir, mat4 transform, float lod, sampler2D sampler)
{
    vec3 envDir = normalize((transform * vec4(dir,0.0)).xyz);
    vec2 uv = mx_latlong_projection(envDir);
    return textureLod(sampler, uv, lod).rgb;
}

// Only GGX is supported for now and the distribution argument is ignored
vec3 mx_environment_radiance(vec3 N, vec3 V, vec3 X, roughnessinfo roughness, int distribution)
{
    vec3 Y = normalize(cross(N, X));
    X = cross(Y, N);

    // Compute shared dot products.
    float NdotV = clamp(dot(N, V), 1e-8, 1.0);
    
    // Integrate outgoing radiance using filtered importance sampling.
    // http://cgg.mff.cuni.cz/~jaroslav/papers/2008-egsr-fis/2008-egsr-fis-final-embedded.pdf
    vec3 radiance = vec3(0.0);
    for (int i = 0; i < u_envSamples; i++)
    {
        vec2 Xi = mx_spherical_fibonacci(i, u_envSamples);

        // Compute the half vector and incoming light direction.
        vec3 H = mx_microfacet_ggx_IS(Xi, X, Y, N, roughness.alphaX, roughness.alphaY);
        vec3 L = -reflect(V, H);
        
        // Compute dot products for this sample.
        float NdotH = clamp(dot(N, H), 1e-8, 1.0);
        float NdotL = clamp(dot(N, L), 1e-8, 1.0);
        float VdotH = clamp(dot(V, H), 1e-8, 1.0);
        float LdotH = VdotH;

        // Sample the environment light from the given direction.
        float pdf = mx_microfacet_ggx_PDF(X, Y, H, NdotH, LdotH, roughness.alphaX, roughness.alphaY);
        float lod = mx_latlong_compute_lod(L, pdf, u_envRadianceMips - 1, u_envSamples);
        vec3 sampleColor = mx_latlong_map_lookup(L, u_envMatrix, lod, u_envRadianceSampler);

        // Compute the geometric term.
        float G = mx_microfacet_ggx_smith_G(NdotL, NdotV, roughness.alpha);
        
        // Fresnel is applied outside the lighting integral for now.
        // TODO: Move Fresnel term into the lighting integral.
        float F = 1.0;

        // Add the radiance contribution of this sample.
        // From https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
        //   incidentLight = sampleColor * NdotL
        //   microfacetSpecular = D * G * F / (4 * NdotL * NdotV)
        //   pdf = D * NdotH / (4 * VdotH)
        //   radiance = incidentLight * microfacetSpecular / pdf
        radiance += sampleColor * G * F * VdotH / (NdotV * NdotH);
    }

    // Normalize and return the final radiance.
    radiance /= float(u_envSamples);
    return radiance;
}

vec3 mx_environment_irradiance(vec3 N)
{
    return mx_latlong_map_lookup(N, u_envMatrix, 0.0, u_envIrradianceSampler);
}

//
// Function to transform uv-coordinates before texture sampling
//
vec2 mx_get_target_uv(vec2 uv)
{
   return uv;
}

int numActiveLightSources()
{
    return min(u_numActiveLightSources, MAX_LIGHT_SOURCES);
}

void sampleLightSource(LightData light, vec3 position, out lightshader result)
{
    result.intensity = vec3(0.0);
}

void mx_roughness_anisotropy(float roughness, float anisotropy, out roughnessinfo result)
{
    result.roughness = roughness;
    result.alpha = clamp(roughness*roughness, M_FLOAT_EPS, 1.0);
    if (anisotropy > 0.0)
    {
        float aspect = sqrt(1.0 - clamp(anisotropy, 0.0, 0.98));
        result.alphaX = min(result.alpha / aspect, 1.0);
        result.alphaY = result.alpha * aspect;
    }
    else
    {
        result.alphaX = result.alpha;
        result.alphaY = result.alpha;
    }
}

void mx_luminance_color3(vec3 _in, vec3 lumacoeffs, out vec3 result)
{
    result = vec3(dot(_in, lumacoeffs));
}


void mx_fresnel_ior(float ior, vec3 N, vec3 V, out float result)
{
    float cosTheta = dot(N, -V);
    result = mx_fresnel_dielectric(cosTheta, ior);
}

// "Artist Friendly Metallic Fresnel", Ole Gulbrandsen, 2014
// http://jcgt.org/published/0003/04/03/paper.pdf

void mx_complex_to_artistic_ior(vec3 ior, vec3 extinction, out vec3 reflectivity, out vec3 edgecolor)
{
    vec3 nm1 = ior - 1.0;
    vec3 np1 = ior + 1.0;
    vec3 k2  = extinction * extinction;
    vec3 r = (nm1*nm1 + k2) / (np1*np1 + k2);
    reflectivity = r;

    vec3 r_sqrt = sqrt(r);
    vec3 n_min = (1.0 - r) / (1.0 + r);
    vec3 n_max = (1.0 + r_sqrt) / (1.0 - r_sqrt);
    edgecolor = (n_max - ior) / (n_max - n_min);
}

void mx_artistic_to_complex_ior(vec3 reflectivity, vec3 edgecolor, out vec3 ior, out vec3 extinction)
{
    vec3 r = clamp(reflectivity, 0.0, 0.99);
    vec3 r_sqrt = sqrt(r);
    vec3 n_min = (1.0 - r) / (1.0 + r);
    vec3 n_max = (1.0 + r_sqrt) / (1.0 - r_sqrt);
    ior = mix(n_max, n_min, edgecolor);

    vec3 np1 = ior + 1.0;
    vec3 nm1 = ior - 1.0;
    vec3 k2 = (np1*np1 * r - nm1*nm1) / (1.0 - r);
    k2 = max(k2, 0.0);
    extinction = sqrt(k2);
}

void mx_conductor_brdf_reflection(vec3 L, vec3 V, float weight, vec3 reflectivity, vec3 edgecolor, roughnessinfo roughness, vec3 N, vec3 X, int distribution, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    float NdotL = dot(N,L);
    float NdotV = dot(N,V);
    if (NdotL <= 0.0 || NdotV <= 0.0)
    {
        result = BSDF(0.0);
        return;
    }

    vec3 Y = normalize(cross(N, X));

    vec3 H = normalize(L + V);
    float NdotH = dot(N, H);

    float D = mx_microfacet_ggx_NDF(X, Y, H, NdotH, roughness.alphaX, roughness.alphaY);
    float G = mx_microfacet_ggx_smith_G(NdotL, NdotV, roughness.alpha);

    vec3 ior_n, ior_k;
    mx_artistic_to_complex_ior(reflectivity, edgecolor, ior_n, ior_k);

    float VdotH = dot(V, H);
    vec3 F = mx_fresnel_conductor(VdotH, ior_n, ior_k);
    F *= weight;

    // Note: NdotL is cancelled out
    result = F * D * G / (4 * NdotV);
}

void mx_conductor_brdf_indirect(vec3 V, float weight, vec3 reflectivity, vec3 edgecolor, roughnessinfo roughness, vec3 N, vec3 X, int distribution, out vec3 result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = vec3(0.0);
        return;
    }

    vec3 ior_n, ior_k;
    mx_artistic_to_complex_ior(reflectivity, edgecolor, ior_n, ior_k);

    vec3 Li = mx_environment_radiance(N, V, X, roughness, distribution);
    vec3 F = mx_fresnel_conductor(dot(N, V), ior_n, ior_k);
    F *= weight;
    result = Li * F;
}

void mx_dielectric_btdf_transmission(vec3 V, float weight, vec3 tint, float ior, roughnessinfo roughness, vec3 normal, vec3 tangent, int distribution, VDF interior, out BSDF result)
{
    // TODO: Attenuate the transmission based on roughness and interior VDF
    result = tint * weight;
}


void mx_diffuse_brdf_reflection(vec3 L, vec3 V, float weight, vec3 color, float roughness, vec3 normal, out BSDF result)
{
    float NdotL = dot(L, normal);
    if (NdotL <= 0.0 || weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    result = color * weight * NdotL * M_PI_INV;
    if (roughness > 0.0)
    {
        result *= mx_orennayar(L, V, normal, NdotL, roughness);
    }
}

void mx_diffuse_brdf_indirect(vec3 V, float weight, vec3 color, float roughness, vec3 normal, out vec3 result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = vec3(0.0);
        return;
    }

    vec3 Li = mx_environment_irradiance(normal);
    result = Li * color * weight;
}

// Fake with simple diffuse transmission
void mx_subsurface_brdf_reflection(vec3 L, vec3 V, float weight, vec3 color, vec3 radius, float anisotropy, vec3 normal, out BSDF result)
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

// Fake with simple diffuse transmission
void mx_subsurface_brdf_indirect(vec3 V, float weight, vec3 color, vec3 radius, float anisotropy, vec3 normal, out vec3 result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = vec3(0.0);
        return;
    }

    // Invert normal since we're transmitting light from the other side
    vec3 Li = mx_environment_irradiance(-normal);
    result = Li * color * weight;
}

// We fake diffuse transmission by using diffuse reflection from the opposite side.
// So this BTDF is really a BRDF.
void mx_diffuse_btdf_reflection(vec3 L, vec3 V, float weight, vec3 color, vec3 normal, out BSDF result)
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

void mx_diffuse_btdf_indirect(vec3 V, float weight, vec3 color, vec3 normal, out vec3 result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = vec3(0.0);
        return;
    }

    // Invert normal since we're transmitting light from the other side
    vec3 Li = mx_environment_irradiance(-normal);
    result = Li * color * weight;
}


void mx_sheen_brdf_reflection(vec3 L, vec3 V, float weight, vec3 color, float roughness, vec3 N, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    float NdotL = dot(N,L);
    float NdotV = dot(N,V);
    if (NdotL <= 0.0 || NdotV <= 0.0)
    {
        result = base;
        return;
    }

    vec3 H = normalize(L + V);
    float NdotH = dot(N, H);

    float alpha = clamp(roughness, M_FLOAT_EPS, 1.0);
    float D = mx_microfacet_sheen_NDF(NdotH, alpha);

    vec3 F = color * weight;

    // Geometry term is skipped and we use a smoother denominator, as in:
    // https://blog.selfshadow.com/publications/s2013-shading-course/rad/s2013_pbs_rad_notes.pdf
    vec3 fr = D * F / (4.0 * (NdotL + NdotV - NdotL*NdotV));

    // Get sheen directional albedo for attenuating base layer
    // in order to be energy conserving.
    float albedo = weight * mx_microfacet_sheen_albedo(NdotV, alpha);

    // We need to include NdotL from the light integral here
    // as in this case it's not cancelled out by the BRDF denominator.
    result = fr * NdotL             // Top layer reflection
           + base * (1.0 - albedo); // Base layer reflection attenuated by top layer
}

void mx_sheen_brdf_indirect(vec3 V, float weight, vec3 color, float roughness, vec3 N, BSDF base, out vec3 result)
{
    if (weight <= 0.0)
    {
        result = base;
        return;
    }

    float NdotV = abs(dot(N,V));
    float alpha = clamp(roughness, M_FLOAT_EPS, 1.0);
    float albedo = weight * mx_microfacet_sheen_albedo(NdotV, alpha);

    vec3 Li = mx_environment_irradiance(N);
    result = Li * color * albedo + base * (1.0 - albedo);
}

void mx_mix_bsdf_reflection(vec3 L, vec3 V, BSDF fg, BSDF bg, float w, out BSDF result)
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

void mx_uniform_edf(vec3 N, vec3 L, vec3 intensity, out EDF result)
{
    result = intensity;
}


void mx_dielectric_brdf_reflection(vec3 L, vec3 V, float weight, vec3 tint, float ior, roughnessinfo roughness, vec3 N, vec3 X, int distribution, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    float NdotL = dot(N,L);
    float NdotV = dot(N,V);
    if (NdotL <= 0.0 || NdotV <= 0.0)
    {
        result = base;
        return;
    }

    vec3 Y = normalize(cross(N, X));

    vec3 H = normalize(L + V);
    float NdotH = dot(N, H);

    float D = mx_microfacet_ggx_NDF(X, Y, H, NdotH, roughness.alphaX, roughness.alphaY);
    float G = mx_microfacet_ggx_smith_G(NdotL, NdotV, roughness.alpha);

    float VdotH = dot(V, H);
    float F = mx_fresnel_schlick(VdotH, ior);
    F *= weight;

    // Note: NdotL is cancelled out
    result = tint * D * G * F / (4 * NdotV) // Top layer reflection
           + base * (1.0 - F);              // Base layer reflection attenuated by top fresnel
}

void mx_dielectric_brdf_transmission(vec3 V, float weight, vec3 tint, float ior, roughnessinfo roughness, vec3 N, vec3 X, int distribution, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    // Dielectric BRDF has no transmission but we must
    // attenuate the base layer transmission by the
    // inverse of top layer reflectance.

    // Abs here to allow transparency through backfaces
    float NdotV = abs(dot(N,V));
    float F = mx_fresnel_schlick(NdotV, ior);
    F *= weight;

    result = base * (1.0 - F); // Base layer transmission attenuated by top fresnel
}

void mx_dielectric_brdf_indirect(vec3 V, float weight, vec3 tint, float ior, roughnessinfo roughness, vec3 N, vec3 X, int distribution, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    vec3 Li = mx_environment_radiance(N, V, X, roughness, distribution);

    float NdotV = dot(N,V);
    float F = mx_fresnel_schlick_roughness(NdotV, ior, roughness.alpha);
    F *= weight;

    result = Li * tint * F     // Top layer reflection
           + base * (1.0 - F); // Base layer reflection attenuated by top fresnel
}

void mx_multiply_bsdf_color_reflection(vec3 L, vec3 V, BSDF in1, vec3 in2, out BSDF result)
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

void IMPL_standard_surface_surfaceshader(float base, vec3 base_color, float diffuse_roughness, float specular, vec3 specular_color, float specular_roughness, float specular_IOR, float specular_anisotropy, float specular_rotation, float metalness, float transmission, vec3 transmission_color, float transmission_depth, vec3 transmission_scatter, float transmission_scatter_anisotropy, float transmission_dispersion, float transmission_extra_roughness, float subsurface, vec3 subsurface_color, vec3 subsurface_radius, float subsurface_scale, float subsurface_anisotropy, float sheen, vec3 sheen_color, float sheen_roughness, bool thin_walled, vec3 normal, vec3 tangent, float coat, vec3 coat_color, float coat_roughness, float coat_anisotropy, float coat_rotation, float coat_IOR, vec3 coat_normal, float coat_affect_color, float coat_affect_roughness, float thin_film_thickness, float thin_film_IOR, float emission, vec3 emission_color, vec3 opacity, out surfaceshader out1)
{
    roughnessinfo coat_roughness_out = roughnessinfo(M_FLOAT_EPS, M_FLOAT_EPS, M_FLOAT_EPS, M_FLOAT_EPS);
    mx_roughness_anisotropy(coat_roughness, coat_anisotropy, coat_roughness_out);
    vec3 geomprop_Tworld_out = normalize(PIX_IN.tangentWorld);
    float coat_affect_roughness_multiply1_out = coat_affect_roughness * coat;
    const float coat_clamped_low_tmp = 0.000000;
    const float coat_clamped_high_tmp = 1.000000;
    float coat_clamped_out = clamp(coat, coat_clamped_low_tmp, coat_clamped_high_tmp);
    float subsurface_selector_out = float(thin_walled);
    vec3 geomprop_Vworld_out = normalize(PIX_IN.positionWorld - u_viewPosition);
    const vec3 coat_attenuation_bg_tmp = vec3(1.000000, 1.000000, 1.000000);
    vec3 coat_attenuation_out = mix(coat_attenuation_bg_tmp, coat_color, coat);
    vec3 emission_weight_out = emission_color * emission;
    vec3 opacity_luminance_out = vec3(0.0);
    mx_luminance_color3(opacity, vec3(0.272287, 0.674082, 0.053689), opacity_luminance_out);
    float coat_affect_roughness_multiply2_out = coat_affect_roughness_multiply1_out * coat_roughness;
    float coat_gamma_multiply_out = coat_clamped_out * coat_affect_color;
    float coat_fresnel_out = 0.0;
    mx_fresnel_ior(coat_IOR, coat_normal, geomprop_Vworld_out, coat_fresnel_out);
    const float coat_affected_rougness_fg_tmp = 1.000000;
    float coat_affected_rougness_out = mix(specular_roughness, coat_affected_rougness_fg_tmp, coat_affect_roughness_multiply2_out);
    const float coat_gamma_in2_tmp = 1.000000;
    float coat_gamma_out = coat_gamma_multiply_out + coat_gamma_in2_tmp;
    const float coat_fresnel_inv_amount_tmp = 1.000000;
    float coat_fresnel_inv_out = coat_fresnel_inv_amount_tmp - coat_fresnel_out;
    roughnessinfo main_rougness_out = roughnessinfo(M_FLOAT_EPS, M_FLOAT_EPS, M_FLOAT_EPS, M_FLOAT_EPS);
    mx_roughness_anisotropy(coat_affected_rougness_out, specular_anisotropy, main_rougness_out);
    vec3 coat_affected_diffuse_color_out = pow(base_color, vec3(coat_gamma_out));
    vec3 coat_affected_subsurface_color_out = pow(subsurface_color, vec3(coat_gamma_out));
    vec3 coat_color_fresnel_out = coat_color * coat_fresnel_inv_out;
    const vec3 coat_emission_attenuation_bg_tmp = vec3(1.000000, 1.000000, 1.000000);
    vec3 coat_emission_attenuation_out = mix(coat_emission_attenuation_bg_tmp, coat_color_fresnel_out, coat);
    vec3 emission_weight_attenuated_out = emission_weight_out * coat_emission_attenuation_out;

    surfaceshader standard_surface_constructor_out = surfaceshader(vec3(0.0),vec3(0.0));
    {
        // Light loop
        vec3 N = normalize(PIX_IN.normalWorld);
        vec3 V = normalize(u_viewPosition - PIX_IN.positionWorld);
        int numLights = numActiveLightSources();
        lightshader lightShader;
        for (int activeLightIndex = 0; activeLightIndex < numLights; ++activeLightIndex)
        {
            sampleLightSource(u_lightData[activeLightIndex], PIX_IN.positionWorld, lightShader);
            vec3 L = lightShader.direction;

            // Calculate the BSDF response for this light source
            BSDF metal_bsdf_out = BSDF(0.0);
            mx_conductor_brdf_reflection(L, V, base, base_color, specular_color, main_rougness_out, normal, tangent, 0, metal_bsdf_out);
            BSDF transmission_bsdf_out = BSDF(0.0);
            BSDF diffuse_bsdf_out = BSDF(0.0);
            mx_diffuse_brdf_reflection(L, V, base, coat_affected_diffuse_color_out, diffuse_roughness, normal, diffuse_bsdf_out);
            BSDF subsurface_bsdf_out = BSDF(0.0);
            mx_subsurface_brdf_reflection(L, V, 1.000000, coat_affected_subsurface_color_out, subsurface_radius, subsurface_anisotropy, normal, subsurface_bsdf_out);
            BSDF translucent_bsdf_out = BSDF(0.0);
            mx_diffuse_btdf_reflection(L, V, 1.000000, coat_affected_subsurface_color_out, normal, translucent_bsdf_out);
            BSDF sheen_bsdf_out = BSDF(0.0);
            mx_sheen_brdf_reflection(L, V, sheen, sheen_color, sheen_roughness, normal, diffuse_bsdf_out, sheen_bsdf_out);
            BSDF selected_subsurface_bsdf_out = BSDF(0.0);
            mx_mix_bsdf_reflection(L, V, translucent_bsdf_out, subsurface_bsdf_out, subsurface_selector_out, selected_subsurface_bsdf_out);
            BSDF subsurface_mix_out = BSDF(0.0);
            mx_mix_bsdf_reflection(L, V, selected_subsurface_bsdf_out, sheen_bsdf_out, subsurface, subsurface_mix_out);
            BSDF transmission_mix_out = BSDF(0.0);
            mx_mix_bsdf_reflection(L, V, transmission_bsdf_out, subsurface_mix_out, transmission, transmission_mix_out);
            BSDF specular_bsdf_out = BSDF(0.0);
            mx_dielectric_brdf_reflection(L, V, specular, specular_color, specular_IOR, main_rougness_out, normal, tangent, 0, transmission_mix_out, specular_bsdf_out);
            BSDF metalness_mix_out = BSDF(0.0);
            mx_mix_bsdf_reflection(L, V, metal_bsdf_out, specular_bsdf_out, metalness, metalness_mix_out);
            BSDF metalness_mix_attenuated_out = BSDF(0.0);
            mx_multiply_bsdf_color_reflection(L, V, metalness_mix_out, coat_attenuation_out, metalness_mix_attenuated_out);
            BSDF coat_bsdf_out = BSDF(0.0);
            mx_dielectric_brdf_reflection(L, V, coat, vec3(1.000000, 1.000000, 1.000000), coat_IOR, coat_roughness_out, coat_normal, geomprop_Tworld_out, 0, metalness_mix_attenuated_out, coat_bsdf_out);

            // Accumulate the light's contribution
            standard_surface_constructor_out.color += lightShader.intensity * coat_bsdf_out;
        }

        // Add surface emission
        {
            EDF emission_edf_out = EDF(0.0);
            mx_uniform_edf(N, V, emission_weight_attenuated_out, emission_edf_out);
            standard_surface_constructor_out.color += emission_edf_out;
        }

        // Add indirect contribution
        {
            BSDF metal_bsdf_out = BSDF(0.0);
            mx_conductor_brdf_indirect(V, base, base_color, specular_color, main_rougness_out, normal, tangent, 0, metal_bsdf_out);
            BSDF transmission_bsdf_out = BSDF(0.0);
            BSDF diffuse_bsdf_out = BSDF(0.0);
            mx_diffuse_brdf_indirect(V, base, coat_affected_diffuse_color_out, diffuse_roughness, normal, diffuse_bsdf_out);
            BSDF subsurface_bsdf_out = BSDF(0.0);
            mx_subsurface_brdf_indirect(V, 1.000000, coat_affected_subsurface_color_out, subsurface_radius, subsurface_anisotropy, normal, subsurface_bsdf_out);
            BSDF translucent_bsdf_out = BSDF(0.0);
            mx_diffuse_btdf_indirect(V, 1.000000, coat_affected_subsurface_color_out, normal, translucent_bsdf_out);
            BSDF sheen_bsdf_out = BSDF(0.0);
            mx_sheen_brdf_indirect(V, sheen, sheen_color, sheen_roughness, normal, diffuse_bsdf_out, sheen_bsdf_out);
            BSDF selected_subsurface_bsdf_out = BSDF(0.0);
            mx_mix_bsdf_indirect(V, translucent_bsdf_out, subsurface_bsdf_out, subsurface_selector_out, selected_subsurface_bsdf_out);
            BSDF subsurface_mix_out = BSDF(0.0);
            mx_mix_bsdf_indirect(V, selected_subsurface_bsdf_out, sheen_bsdf_out, subsurface, subsurface_mix_out);
            BSDF transmission_mix_out = BSDF(0.0);
            mx_mix_bsdf_indirect(V, transmission_bsdf_out, subsurface_mix_out, transmission, transmission_mix_out);
            BSDF specular_bsdf_out = BSDF(0.0);
            mx_dielectric_brdf_indirect(V, specular, specular_color, specular_IOR, main_rougness_out, normal, tangent, 0, transmission_mix_out, specular_bsdf_out);
            BSDF metalness_mix_out = BSDF(0.0);
            mx_mix_bsdf_indirect(V, metal_bsdf_out, specular_bsdf_out, metalness, metalness_mix_out);
            BSDF metalness_mix_attenuated_out = BSDF(0.0);
            mx_multiply_bsdf_color_indirect(V, metalness_mix_out, coat_attenuation_out, metalness_mix_attenuated_out);
            BSDF coat_bsdf_out = BSDF(0.0);
            mx_dielectric_brdf_indirect(V, coat, vec3(1.000000, 1.000000, 1.000000), coat_IOR, coat_roughness_out, coat_normal, geomprop_Tworld_out, 0, metalness_mix_attenuated_out, coat_bsdf_out);

            standard_surface_constructor_out.color += coat_bsdf_out;
        }

        standard_surface_constructor_out.transparency = vec3(0.0);
    }

    out1 = standard_surface_constructor_out;
}

// TODO: Arguments here manually added. Need to modify generator to place uniforms here
// versus as global.
vec3 main_function(
   float base,
   vec3 base_color,
   float diffuse_roughness,
   float specular,
   vec3 specular_color,
   float specular_roughness,
   float specular_IOR,
   float specular_anisotropy,
   float specular_rotation,
   float metalness,
   float transmission,
   vec3 transmission_color,
   float transmission_depth,
   vec3 transmission_scatter,
   float transmission_scatter_anisotropy,
   float transmission_dispersion,
   float transmission_extra_roughness,
   float subsurface,
   vec3 subsurface_color,
   vec3 subsurface_radius,
   float subsurface_scale,
   float subsurface_anisotropy,
   float sheen,
   vec3 sheen_color,
   float sheen_roughness,
   bool thin_walled,
   float coat,
   vec3 coat_color,
   float coat_roughness,
   float coat_anisotropy,
   float coat_rotation,
   float coat_IOR,
   float coat_affect_color,
   float coat_affect_roughness,
   float thin_film_thickness,
   float thin_film_IOR,
   float emission,
   vec3 emission_color,
   vec3 opacity   
)
{
    vec3 geomprop_Nworld_out = normalize(PIX_IN.normalWorld);
    vec3 geomprop_Tworld_out = normalize(PIX_IN.tangentWorld);

    surfaceshader SR_default_out = surfaceshader(vec3(0.0),vec3(0.0));
    IMPL_standard_surface_surfaceshader(base, base_color, diffuse_roughness, specular, specular_color, specular_roughness, specular_IOR, specular_anisotropy, specular_rotation, metalness, transmission, transmission_color, transmission_depth, transmission_scatter, transmission_scatter_anisotropy, transmission_dispersion, transmission_extra_roughness, subsurface, subsurface_color, subsurface_radius, subsurface_scale, subsurface_anisotropy, sheen, sheen_color, sheen_roughness, thin_walled, geomprop_Nworld_out, geomprop_Tworld_out, coat, coat_color, coat_roughness, coat_anisotropy, coat_rotation, coat_IOR, geomprop_Nworld_out, coat_affect_color, coat_affect_roughness, thin_film_thickness, thin_film_IOR, emission, emission_color, opacity, SR_default_out);
    return SR_default_out.color;
}


mayaSurfaceShaderOutput mayaSurfaceShaderDiffuse(
	vec3 outColor, 
	vec3 outTransparency, 
	vec3 outGlowColor, 
	vec3 outMatteOpacity) 
{ 
	mayaSurfaceShaderOutput result; 
	result.outColor = outColor; 
	if(!mayaAlphaCut) {
		result.outColor *= saturate(1.0f - outTransparency); 
	} 
	result.outTransparency = outTransparency; 
	result.outGlowColor = outGlowColor; 
	result.outMatteOpacity = outMatteOpacity; 
	return result; 
} 

vec3 Pw( in vec3 ipw )
{ 
    return ipw; 
} 

vec4 mayaFogDepth(vec3 Pw, mat4x4 ViewProj, float fogStart, float fogEnd, int fogMode, float fogDensity) 
{ 
    vec4 pc = mul( ViewProj, vec4(Pw,1.0f) ); 
    float distanceToCamera = abs(pc.w); 
    float fogFactor = 0.0f;  
    if (fogMode == 0) { 
		fogFactor = saturate((fogEnd - distanceToCamera) / (fogEnd - fogStart)); 
    } 
    else if (fogMode == 1) { 
		fogFactor = 1.0f / exp(distanceToCamera * fogDensity); 
    } 
    else if (fogMode == 2) { 
		fogFactor = 1.0f / exp(pow(distanceToCamera * fogDensity, 2)); 
    } 
    return vec4(fogFactor, fogFactor, fogFactor, 1.0f); 
} 

mayaSurfaceShaderOutput mayaComputeSurfaceFinal(
	mayaSurfaceShaderOutput input_is_glsl_kw, 
	float extraOpacity, 
	bool fogEnabled, 
	vec4 fogDepthSurface, 
	vec4 fogColor, 
	float fogMultiplier) 
{ 
	const vec3 intenseVec = vec3(0.3333, 0.3333, 0.3333); 
	vec3 opacity = saturate(1.0f - input_is_glsl_kw.outTransparency); 
	if (fogEnabled) { 
		float fogFactor = (1.0f - fogDepthSurface.x) * fogColor.a * fogMultiplier; 
		float opacityAvg = (opacity.x + opacity.y + opacity.z) / 3; 
		fogColor *= vec4(opacityAvg, opacityAvg, opacityAvg, 1.0f); 
		input_is_glsl_kw.outColor =  lerp(input_is_glsl_kw.outColor, fogColor.xyz, fogFactor); 
	} 
	input_is_glsl_kw.outSurfaceFinal = extraOpacity * vec4(input_is_glsl_kw.outColor, dot(opacity, intenseVec)); 
	return input_is_glsl_kw; 
} 


vec4 mayaSurfaceShaderOutputTooutSurfaceFinal(mayaSurfaceShaderOutput input_is_glsl_kw) 
{ 
	return input_is_glsl_kw.outSurfaceFinal; 
} 


 // Pixel Shader 
void main()
{ 

     // ShaderBody 
    mayaSurfaceShaderOutput s_mayaSurfaceShaderOutput4696; 
    vec3 v_out4702 = main_function( base, base_color, diffuse_roughness, specular, specular_color, specular_roughness, specular_IOR, specular_anisotropy, specular_rotation, metalness, transmission, transmission_color, transmission_depth, transmission_scatter, transmission_scatter_anisotropy, transmission_dispersion, transmission_extra_roughness, subsurface, subsurface_color, subsurface_radius, subsurface_scale, subsurface_anisotropy, sheen, sheen_color, sheen_roughness, thin_walled, coat, coat_color, coat_roughness, coat_anisotropy, coat_rotation, coat_IOR, coat_affect_color, coat_affect_roughness, thin_film_thickness, thin_film_IOR, emission, emission_color, opacity ); 
    mayaSurfaceShaderOutput v_mayaSurfaceShaderDiffuse4698 = mayaSurfaceShaderDiffuse( v_out4702, surfaceShader_1outTransparency, surfaceShader_1outGlowColor, surfaceShader_1outMatteOpacity ); 
    vec3 v_Pw4706 = Pw( PIX_IN.Pw ); 
    vec4 v_maya_FogDepthSurface4699 = mayaFogDepth( v_Pw4706, ViewProj, fogStart, fogEnd, fogMode, fogDensity ); 
    s_mayaSurfaceShaderOutput4696 = mayaComputeSurfaceFinal( v_mayaSurfaceShaderDiffuse4698, extraOpacity, fogEnabled, v_maya_FogDepthSurface4699, fogColor, fogMultiplier ); 
    vec4 v_outSurfaceFinal4691 = mayaSurfaceShaderOutputTooutSurfaceFinal( s_mayaSurfaceShaderOutput4696 ); 

ColorOut = v_outSurfaceFinal4691;
} 
}


/////////////////////// Techniques /////// 
technique Main
{
    pass P0
    {
        VertexShader ( in vertexInS , out vertOutS VS_OUT ) = reqdVtxFormat_VS;
        PixelShader ( in fragInS PIX_IN , out fragOutS ) = reqdVtxFormat;
    }
}
