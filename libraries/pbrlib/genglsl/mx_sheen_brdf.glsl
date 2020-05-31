#include "pbrlib/genglsl/lib/mx_microfacet_sheen.glsl"

void mx_sheen_brdf_reflection(vec3 L, vec3 V, float weight, vec3 color, float roughness, vec3 N, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    vec3 H = normalize(L + V);

    float NdotL = clamp(dot(N, L), M_FLOAT_EPS, 1.0);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);
    float NdotH = clamp(dot(N, H), M_FLOAT_EPS, 1.0);

    float alpha = clamp(roughness, M_FLOAT_EPS, 1.0);
    float D = mx_imageworks_sheen_NDF(NdotH, alpha);

    // Geometry term is skipped and we use a smoother denominator, as in:
    // https://blog.selfshadow.com/publications/s2013-shading-course/rad/s2013_pbs_rad_notes.pdf
    vec3 fr = D * color / (4.0 * (NdotL + NdotV - NdotL*NdotV));

    float dirAlbedo = mx_imageworks_sheen_directional_albedo(NdotV, alpha);

    // We need to include NdotL from the light integral here
    // as in this case it's not cancelled out by the BRDF denominator.
    result = fr * NdotL * weight                // Top layer reflection
           + base * (1.0 - dirAlbedo * weight); // Base layer reflection attenuated by top layer
}

void mx_sheen_brdf_indirect(vec3 V, float weight, vec3 color, float roughness, vec3 N, BSDF base, out vec3 result)
{
    if (weight <= 0.0)
    {
        result = base;
        return;
    }

    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    float alpha = clamp(roughness, M_FLOAT_EPS, 1.0);
    float dirAlbedo = mx_imageworks_sheen_directional_albedo(NdotV, alpha);

    vec3 Li = mx_environment_irradiance(N);
    result = Li * color * dirAlbedo * weight        // Top layer reflection
             + base * (1.0 - dirAlbedo * weight);   // Base layer reflection attenuated by top layer
}
