#include "pbrlib/genglsl/lib/mx_microfacet.glsl"

// http://www.aconty.com/pdf/s2017_pbs_imageworks_sheen.pdf
// Equation 2
float mx_imageworks_sheen_NDF(float cosTheta, float roughness)
{
    float invRoughness = 1.0 / max(roughness, 0.0001);
    float cos2 = cosTheta * cosTheta;
    float sin2 = 1.0 - cos2;
    return (2.0 + invRoughness) * pow(sin2, invRoughness * 0.5) / (2.0 * M_PI);
}

// Rational curve fit approximation for the directional albedo of Imageworks sheen.
float mx_imageworks_sheen_directional_albedo(float cosTheta, float roughness)
{
    float a = 1.59053 - 2.00439 * cosTheta + 3.45331 * roughness;
    float b = 1.0 + 7.99160 * cosTheta + 8.25488 * roughness;
    return clamp(a / b, 0.0, 1.0);
}
