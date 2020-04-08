#include "pbrlib/genglsl/lib/mx_bsdfs.glsl"

void mx_generalized_schlick_brdf_reflection(vec3 L, vec3 V, float weight, vec3 color0, vec3 color90, float exponent, vec2 roughness, vec3 N, vec3 X, int distribution, BSDF base, out BSDF result)
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
    float VdotH = dot(V, H);

    float D = mx_microfacet_ggx_NDF(X, Y, H, NdotH, roughness.x, roughness.y);
    vec3 F = mx_fresnel_schlick(VdotH, color0, color90, exponent);
    float G = mx_microfacet_ggx_smith_G(NdotL, NdotV, mx_average_roughness(roughness));

    vec3 dirAlbedo = mx_microfacet_ggx_directional_albedo(NdotV, mx_average_roughness(roughness), color0, color90);
    float avgDirAlbedo = dot(dirAlbedo, vec3(1.0 / 3.0));

    // Note: NdotL is cancelled out
    result = D * F * G * weight / (4 * NdotV)		// Top layer reflection
           + base * (1.0 - avgDirAlbedo * weight);	// Base layer reflection attenuated by top directional albedo
}

void mx_generalized_schlick_brdf_transmission(vec3 V, float weight, vec3 color0, vec3 color90, float exponent, vec2 roughness, vec3 N, vec3 X, int distribution, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    // Glossy BRDF has no transmission but we must 
    // attenuate the base layer transmission by the 
    // inverse of top layer reflectance.

    // Abs here to allow transparency through backfaces
    float NdotV = abs(dot(N, V)); 
    vec3 dirAlbedo = mx_microfacet_ggx_directional_albedo(NdotV, mx_average_roughness(roughness), color0, color90);
    float avgDirAlbedo = dot(dirAlbedo, vec3(1.0 / 3.0));

    result = base * (1.0 - avgDirAlbedo * weight); // Base layer transmission attenuated by top directional albedo
}

void mx_generalized_schlick_brdf_indirect(vec3 V, float weight, vec3 color0, vec3 color90, float exponent, vec2 roughness, vec3 N, vec3 X, int distribution, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    vec3 Li = mx_environment_radiance(N, V, X, roughness, color0, color90, distribution);

    float NdotV = dot(N, V);
    vec3 dirAlbedo = mx_microfacet_ggx_directional_albedo(NdotV, mx_average_roughness(roughness), color0, color90);
    float avgDirAlbedo = dot(dirAlbedo, vec3(1.0 / 3.0));

    result = Li * weight                            // Top layer reflection
           + base * (1.0 - avgDirAlbedo * weight);  // Base layer reflection attenuated by top directional albedo
}
