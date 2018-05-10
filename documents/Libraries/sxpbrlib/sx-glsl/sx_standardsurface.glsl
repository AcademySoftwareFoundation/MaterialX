#include "sxpbrlib/sx-glsl/sx_diffusebsdf.glsl"
#include "sxpbrlib/sx-glsl/sx_coatingbsdf.glsl"

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
    vec3 N = sx_front_facing(normal);
    vec3 V = normalize(u_viewPosition - vd.positionWorld);

    result.color = vec3(0.0);
    result.transparency = vec3(1.0);

    //
    // Compute direct lighting
    //

    lightshader lightShader;
    int numLights = numActiveLightSources();
    for (int activeLightIndex = 0; activeLightIndex < numLights; ++activeLightIndex)
    {
        sampleLightSource(u_lightData[activeLightIndex], vd.positionWorld, lightShader);

        vec3 L = lightShader.direction;

        BSDF diffuse_bsdf;
        sx_diffusebsdf(L, V, base * base_color, diffuse_roughness, N, diffuse_bsdf);

        BSDF specular_bsdf;
        sx_coatingbsdf(L, V, specular_color*specular, specular_IOR, specular_roughness, specular_anisotropy, N, tangent, 0, diffuse_bsdf, specular_bsdf);

        result.color += lightShader.intensity * specular_bsdf.fr;
    }

    //
    // Compute indirect lighting
    //

    if (specular > 0.0)
    {
        float F = sx_fresnel_schlick_roughness(dot(N, V), specular_IOR, specular_roughness);
        vec3 specularEnv = sx_environment_specular(N, V, specular_roughness);
        result.color += specular_color * specular * F * specularEnv;
    }

    // TODO: Indirect diffuse
    // vec3 irradianceEnv = sx_environment_irradiance(N);

    // Add emission
    result.color += emission_color * emission;
}
