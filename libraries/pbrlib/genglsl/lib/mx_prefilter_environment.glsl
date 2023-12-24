#include "mx_microfacet_specular.glsl"

// This is the inverse calculation of `mx_latlong_compute_lod` in `mx_environment_prefilter.glsl`.
float mx_lod_to_alpha(float lod)
{
    float lodBias = lod / float($envRadianceMips);
    if (lodBias < 0.5)
    {
        return lodBias * lodBias;
    }
    else
    {
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
  
vec3 spherical_to_cartesian(float phi, float cosTheta)
{
    float sinPhi = sin(phi);
    float cosPhi = cos(phi);
    float sinTheta = sqrt(clamp(1.0 - cosTheta * cosTheta, 0.0, 1.0));
    float x = cosPhi * sinTheta;
    float y = sinPhi * sinTheta;
    float z = cosTheta;
    return vec3(x, y, z);
}

void sample_ggx_dir(vec2 u, vec3 V, mat3 localToWorld, float alpha, out vec3 L,
                    out float NdotL, out float NdotH, out float VdotH)
{
    // GGX NDF sampling
    float cosTheta = sqrt((1.0 - u.x) / (1.0 + (alpha * alpha - 1.0) * u.x));
    float phi = M_PI * 2.0 * u.y;

    vec3 localH = spherical_to_cartesian(phi, cosTheta);

    NdotH = cosTheta;

    vec3 localV;

    // localV == localN
    localV = vec3(0.0, 0.0, 1.0);
    VdotH  = NdotH;

    // Compute { localL = reflect(-localV, localH) }
    vec3 localL = -localV + 2.0 * VdotH * localH;
    NdotL = localL.z;

    L = localToWorld * localL;
}

vec3 mx_prefilter_environment()
{
    vec2 uv = gl_FragCoord.xy * pow(2.0, $envPrefilterMip) / vec2(2048.0, 1024.0);
    if ($envPrefilterMip == 0)
    {
        return textureLod($envRadiance, uv, 0).rgb;
    }

    // Do an inverse projection, i.e. go from equiangular coordinates to cartesian coordinates
    vec3 N = mx_latlong_map_projection_inverse(uv);

    mat3 localToWorld = get_local_frame(N);
    vec3 V = N;
    float NdotV = 1; // Because N == V
    float alpha = mx_lod_to_alpha(float($envPrefilterMip));
    // If we use prefiltering, we can have a smaller sample count, since pre-filtering will reduce
    // the variance of the samples by choosing higher mip levels where necessary. We haven't
    // implemented prefiltering yet, so we use a high sample count.
    int sampleCount = 1597; // Must be a Fibonacci number

    vec3 lightInt = vec3(0.0, 0.0, 0.0);
    float cbsdfInt = 0.0;

    for (int i = 0; i < sampleCount; ++i)
    {
        vec2 Xi = mx_spherical_fibonacci(i, sampleCount);

        vec3 L;
        float NdotL;
        float NdotH;
        float LdotH;
        sample_ggx_dir(Xi, V, localToWorld, alpha, L, NdotL, NdotH, LdotH);
        if (NdotL <= 0) continue; // Note that some samples will have 0 contribution

        // If we were to implement pre-filtering, we would do so here.
        float mipLevel = 0;
        vec3 val = mx_latlong_map_lookup(L, $envMatrix, mipLevel, $envRadiance);
        const float F = 1.0;
        float G = mx_ggx_smith_G2(NdotL, NdotV, alpha);
        lightInt += F * G * val;
        cbsdfInt += F * G;
    }

    return lightInt / cbsdfInt;
}
