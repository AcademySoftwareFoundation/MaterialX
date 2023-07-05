#include "mx_microfacet_specular.glsl"

// This is the inverse calculation of `mx_latlong_compute_lod` in `mx_environment_prefilter.glsl`.
float mx_lod_to_alpha(float lod)
{
    float lodBias = lod / float($envRadianceMips);
    if (lodBias < 0.5) {
        return lodBias * lodBias;
    } else {
        return 2.0 * (lodBias - 0.375);
    }
}

// Generates an orthonormal (row-major) basis from a unit vector.
// The resulting rotation matrix has the determinant of +1.
// Ref: 'ortho_basis_pixar_r2' from http://marc-b-reynolds.github.io/quaternions/2016/07/06/Orthonormal.html
mat3 get_local_frame(vec3 localZ)
{
    float x  = localZ.x;
    float y  = localZ.y;
    float z  = localZ.z;
    float sz = (z < 0) ? -1.0 : 1.0;
    float a  = 1 / (sz + z);
    float ya = y * a;
    float b  = x * ya;
    float c  = x * sz;

    vec3 localX = vec3(c * x * a - 1, sz * b, c);
    vec3 localY = vec3(b, y * ya - sz, y);

    // Note: due to the quaternion formulation, the generated frame is rotated by 180 degrees,
    // s.t. if localZ = {0, 0, 1}, then localX = {-1, 0, 0} and localY = {0, -1, 0}.
    return mat3(localX, localY, localZ);
}
  
// Precompute part of lambdaV
float get_smith_joint_ggx_part_lambda_v(float NdotV, float alpha)
{
    float a2 = alpha * alpha;
    return sqrt((-NdotV * a2 + NdotV) * NdotV + a2);
}

vec3 spherical_to_cartesian(float phi, float cosTheta)
{
    float sinPhi = sin(phi);
    float cosPhi = cos(phi);
    float sinTheta = sqrt(clamp(1.0 - cosTheta * cosTheta, 0.0, 1.0));
    float x = cosPhi * sinTheta;
    float y = sinPhi * sinTheta;
    float z = cosTheta;
    // float x = -cosTheta * sinPhi;
    // float y = -sinTheta;
    // float z = cosTheta * cosPhi;
    return vec3(x, y, z);
}

void sample_ggx_dir(
    vec2 u,
    vec3 V,
    mat3 localToWorld,
    float alpha,
    out vec3 L,
    out float NdotL,
    out float NdotH,
    out float VdotH,
    bool VeqN = false
)
{
    // GGX NDF sampling
    float cosTheta = sqrt((1.0 - u.x) / (1.0 + (alpha * alpha - 1.0) * u.x));
    float phi = M_PI * 2.0 * u.y;

    vec3 localH = spherical_to_cartesian(phi, cosTheta);

    NdotH = cosTheta;

    vec3 localV;

    if (VeqN)
    {
        // localV == localN
        localV = vec3(0.0, 0.0, 1.0);
        VdotH  = NdotH;
    }
    else
    {
        localV = transpose(localToWorld) * V;
        VdotH  = saturate(dot(localV, localH));
    }

    // Compute { localL = reflect(-localV, localH) }
    vec3 localL = -localV + 2.0 * VdotH * localH;
    NdotL = localL.z;

    L = localToWorld * localL;
}

// Note: V = G / (4 * NdotL * NdotV)
// Ref: http://jcgt.org/published/0003/02/03/paper.pdf
float v_smith_joint_ggx(float NdotL, float NdotV, float alpha, float partLambdaV)
{
    // TODO: use mx_ggx_smith_G2 instead.
    float a2 = alpha * alpha;

    // Original formulation:
    // lambda_v = (-1 + sqrt(a2 * (1 - NdotL2) / NdotL2 + 1)) * 0.5
    // lambda_l = (-1 + sqrt(a2 * (1 - NdotV2) / NdotV2 + 1)) * 0.5
    // G        = 1 / (1 + lambda_v + lambda_l);

    // Reorder code to be more optimal:
    float lambdaV = NdotL * partLambdaV;
    float lambdaL = NdotV * sqrt((-NdotL * a2 + NdotL) * NdotL + a2);

    // Simplify visibility term: (2.0 * NdotL * NdotV) /  ((4.0 * NdotL * NdotV) * (lambda_v + lambda_l))
    return 0.5 / max(lambdaV + lambdaL, FLT_MIN);
}

// // Use lower MIP-map levels for fetching samples with low probabilities
// // in order to reduce the variance.
// // Ref: http://http.developer.nvidia.com/GPUGems3/gpugems3_ch20.html
// float get_prefiltered_mip_level(float NdotH, float alpha, float sampleCount, vec3 L)
// {
//     // OmegaS: Solid angle associated with the sample
//     // OmegaP: Solid angle associated with the texel of the cubemap
//     float omegaS;

//     // float PDF = D * NdotH * Jacobian, where Jacobian = 1 / (4 * LdotH).
//     // Since (N == V), NdotH == LdotH.
//     // Thus  PDF = D / 4.
//     float a2 = alpha * alpha;
//     float s = (NdotH * a2 - NdotH) * NdotH + 1.0;
//     float d_GGX = M_PI_INV * a2 / s * s;
//     float pdf = d_GGX * 0.25;
//     omegaS = 1.0 / (sampleCount * pdf);

//     const float mipBias = alpha;

//     vec3 envDir = normalize(($envMatrix * vec4(L,0.0)).xyz);
//     vec2 uv = mx_latlong_projection(envDir);
//     float deltaLongitude = 2.0 * M_PI / 2048.0;
//     float deltaLatitude = M_PI / 1024.0;
//     float latitudeStart = (uv.y - 0.5) * M_PI - 0.5 * deltaLatitude;
//     float latitudeEnd = latitudeStart + deltaLatitude;
//     float omegaP = deltaLongitude * (sin(latitudeEnd) - sin(latitudeStart));
//     float invOmegaP = 1.0 / omegaP;

//     return 0.5 * log2(omegaS * invOmegaP) + mipBias;
// }

vec3 mx_pre_convolve_environment()
{
    vec2 uv = gl_FragCoord.xy * pow(2.0, (float)$convolutionMipLevel) / vec2(2048.0, 1024.0);
    if ($convolutionMipLevel == 0) {
        return textureLod($envRadiance, uv, 0).rgb;
    }

    // Do an inverse projection, i.e. go from equiangular coordinates to cartesian coordinates
    vec3 N = mx_latlong_map_lookup_inverse(uv);

    mat3 localToWorld = get_local_frame(N);
    const vec3 V = N;
    const float NdotV = 1; // Because N == V
    float alpha = mx_lod_to_alpha(float($convolutionMipLevel));
    float partLambdaV =  get_smith_joint_ggx_part_lambda_v(NdotV, alpha);
    // int sampleCount = 89; // Must be a Fibonacci number
    int sampleCount = 144; // Must be a Fibonacci number

    float3 lightInt = float3(0.0, 0.0, 0.0);
    float  cbsdfInt = 0.0;

    for (int i = 0; i < sampleCount; ++i)
    {
        vec2 Xi = mx_spherical_fibonacci(i, sampleCount);

        vec3 L;
        float NdotL;
        float NdotH;
        float LdotH;
        sample_ggx_dir(Xi, V, localToWorld, alpha, L, NdotL, NdotH, LdotH, true);

        float mipLevel = 0.0; // get_prefiltered_mip_level(NdotH, alpha, sampleCount, L);
        vec3 val = mx_latlong_map_lookup(L, $envMatrix, mipLevel, $envRadiance);
        const float F = 1.0;
        float G = v_smith_joint_ggx(NdotL, NdotV, alpha, partLambdaV) * NdotL * NdotV; // 4 cancels out
        lightInt += F * G * val;
        cbsdfInt += F * G;
    }

    return lightInt / cbsdfInt;
}
