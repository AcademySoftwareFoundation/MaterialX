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
        BSDF substrate_bsdf = BSDF(vec3(0.0), vec3(0.0));
        if (metalness < 1.0 - M_FLOAT_EPS)
        {
            BSDF diffuse_bsdf = BSDF(vec3(0.0), vec3(0.0));
            if (subsurface < 1.0f - M_FLOAT_EPS)
            {
                sx_diffusebsdf(L, V, base_color, diffuse_roughness, normal, diffuse_bsdf);
            }
            BSDF sss_bsdf = BSDF(vec3(0.0), vec3(0.0));
            if (subsurface > M_FLOAT_EPS)
            {
                sx_translucentbsdf(L, V, subsurface_color, normal, sss_bsdf);
            }
            substrate_bsdf.fr = mix(diffuse_bsdf.fr, sss_bsdf.fr, subsurface);

            if (need_specular)
            {
                sx_coatingbsdf(L, V, specular_color, specular_IOR, specular_roughness, specular_anisotropy, normal, tangent, 0, substrate_bsdf, substrate_bsdf);
            }
        }

        // Metal component
        BSDF metal_bsdf = BSDF(vec3(0.0), vec3(0.0));
        if (metalness > M_FLOAT_EPS)
        {
           sx_metalbsdf(L, V, ior_n, ior_k, specular_roughness, specular_anisotropy, normal, tangent, 0, metal_bsdf);
        }

        BSDF total_bsdf = BSDF(vec3(0.0), vec3(0.0));
        total_bsdf.fr = mix(substrate_bsdf.fr, metal_bsdf.fr, metalness);

        if (coat > M_FLOAT_EPS)
        {
            total_bsdf.fr *= coat_color; // scale by coating "transmission color"
            sx_coatingbsdf(L, V, vec3(coat), coat_IOR, coat_roughness, 0.0, coat_normal, tangent, 0, total_bsdf, total_bsdf);
        }

        result.color += lightShader.intensity * total_bsdf.fr;
    }

    //
    // Compute indirect lighting
    //
    
    vec3 coat_attenuation = mix(vec3(1.0), coat_color, coat);
    float metalness_attenuation = 1.0 - metalness;

    // Add emission
    result.color += emission_color * emission * coat_attenuation;

    vec3 Li_diffuse = sx_environment_irradiance(normal) * coat_attenuation * metalness_attenuation;
    result.color +=  Li_diffuse * base_color;

    vec3 Li_specular = sx_environment_specular(normal, V, specular_roughness) * coat_attenuation;
    if (need_specular)
    {
        vec3 F = specular_color * sx_fresnel_schlick_roughness(dot(normal, V), specular_IOR, specular_roughness);
        result.color += Li_specular * F * metalness_attenuation;
    }
    if (metalness > M_FLOAT_EPS)
    {
        vec3 F = sx_fresnel_conductor(dot(normal, V), ior_n, ior_k);
        result.color += Li_specular * F * metalness;
    }
    if (coat > M_FLOAT_EPS)
    {
        vec3 Li_coat = sx_environment_specular(coat_normal, V, coat_roughness);
        float F = sx_fresnel_schlick_roughness(dot(coat_normal, V), coat_IOR, coat_roughness);
        result.color +=  F * Li_coat * coat;
    }
}
