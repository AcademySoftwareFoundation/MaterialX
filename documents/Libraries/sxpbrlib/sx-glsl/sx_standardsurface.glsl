#include "sxpbrlib/sx-glsl/sx_diffusebsdf.glsl"
#include "sxpbrlib/sx-glsl/sx_coatingbsdf.glsl"
#include "sxpbrlib/sx-glsl/sx_metalbsdf.glsl"
#include "sxpbrlib/sx-glsl/sx_translucentbsdf.glsl"
#include "sxpbrlib/sx-glsl/sx_complexior.glsl"

// TODO: Support more lobes:
// metal, coat, transmission, SSS, thin film
void sx_standardsurface(
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
    bool thin_walled,
    vec3 normal,
    vec3 tangent,
    float coat,
    vec3 coat_color,
    float coat_roughness,
    float coat_IOR,
    vec3 coat_normal,
    float thin_film_thickness,
    float thin_film_IOR,
    float emission,
    vec3 emission_color,
    vec3 opacity,
    bool caustics,
    bool internal_reflections,
    bool exit_to_background,
    float indirect_diffuse,
    float indirect_specular,
    out surfaceshader result)
{
    vec3 V = normalize(u_viewPosition - vd.positionWorld);

    result.color = vec3(0.0);
    result.transparency = vec3(1.0);

    base_color *= base;
    specular_color *= specular;

    bool need_specular = !sx_is_tiny(specular_color);

    vec3 coat_attenuation = mix(vec3(1.0), coat_color, coat);

    vec3 ior_n, ior_k;
    sx_complexior(base_color, specular_color, ior_n, ior_k);

    //
    // Compute direct lighting
    //

    lightshader lightShader;
    int numLights = numActiveLightSources();
    for (int activeLightIndex = 0; activeLightIndex < numLights; ++activeLightIndex)
    {
        sampleLightSource(u_lightData[activeLightIndex], vd.positionWorld, lightShader);

        vec3 L = lightShader.direction;

        // Diffuse and dielectric components

        BSDF diffuse_bsdf = BSDF(0.0);
        sx_diffusebsdf(L, V, base_color, diffuse_roughness, normal, diffuse_bsdf);

        BSDF sss_bsdf = BSDF(0.0);
        sx_translucentbsdf(L, V, subsurface_color, normal, sss_bsdf);

        BSDF substrate_bsdf = BSDF(0.0);
        substrate_bsdf = mix(sss_bsdf, diffuse_bsdf, subsurface);

        sx_coatingbsdf(L, V, specular_color, specular_IOR, specular_roughness, specular_anisotropy, normal, tangent, 0, substrate_bsdf, substrate_bsdf);

        // Metal component

        BSDF metal_bsdf = BSDF(0.0);
        sx_metalbsdf(L, V, ior_n, ior_k, specular_roughness, specular_anisotropy, normal, tangent, 0, metal_bsdf);

        BSDF total_bsdf;
        total_bsdf = mix(substrate_bsdf, metal_bsdf, metalness);

        // Clear-coat component

        total_bsdf *= coat_attenuation;
        sx_coatingbsdf(L, V, vec3(coat), coat_IOR, coat_roughness, 0.0, coat_normal, tangent, 0, total_bsdf, total_bsdf);

        result.color += lightShader.intensity * total_bsdf;
    }

    //
    // Compute indirect lighting
    //

    // Add emission
    result.color += emission_color * emission * coat_attenuation;

    vec3 Li_diffuse_bsdf;
    sx_diffusebsdf_ibl(V, base_color, diffuse_roughness, normal, Li_diffuse_bsdf);

    vec3 Li_sss_bsdf;
    sx_translucentbsdf_ibl(V, subsurface_color, normal, Li_sss_bsdf);

    vec3 Li_substrate_bsdf;
    Li_substrate_bsdf = mix(Li_diffuse_bsdf, Li_sss_bsdf, subsurface);

    sx_coatingbsdf_ibl(V, specular_color, specular_IOR, specular_roughness, specular_anisotropy, normal, tangent, 0, Li_substrate_bsdf, Li_substrate_bsdf);

    vec3 Li_metal_bsdf;
    sx_metalbsdf_ibl(V, ior_n, ior_k, specular_roughness, specular_anisotropy, normal, tangent, 0, Li_metal_bsdf);

    vec3 Li_total_bsdf;
    Li_total_bsdf = mix(Li_substrate_bsdf, Li_metal_bsdf, metalness);

    Li_total_bsdf *= coat_attenuation;
    sx_coatingbsdf_ibl(V, vec3(coat), coat_IOR, coat_roughness, 0.0, coat_normal, tangent, 0, Li_total_bsdf, Li_total_bsdf);

    result.color += Li_total_bsdf;
}
