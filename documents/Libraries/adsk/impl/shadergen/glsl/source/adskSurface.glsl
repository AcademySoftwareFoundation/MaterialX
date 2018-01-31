#include "adsk/impl/shadergen/glsl/source/adsk_shadingmodel_functions.glsl"

void adskSurface(
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
    vec3 worldNormal = normalize(normal);

    // Compute diffuse, and texture map with a checker
    vec3 _diffuse = vec3(0.0);
    _diffuse = computeDiffuse(diffuse_roughness, worldNormal);

    // Compute specular
    vec3 worldView = normalize(PS_IN.WorldView); // Not correct. Which one to use
    vec3 _specular = vec3(0.0);
    computeSpecular(specular_roughness, worldNormal, worldView, _specular);

    // Extra inputs
    float directDiffuse = 1.0;
    float directSpecular = 1.0;
    bool specularFresnel = false;
    bool FresnelAffectDiffuse = false;
    bool FresnelUseIOR = false;
    float Ksn = 0.0;
    vec3 IrradianceEnv = IrradianceEnvironment(worldNormal);
    vec3 SpecularEnv = SpecularEnvironment(worldNormal, worldView, specular_roughness);

    // Compute total bsdf
    standardShaderCombiner(
        _diffuse,
        _specular,
        base_color,
        diffuse_roughness,
        specular_color,
        specular_roughness,
        metalness,
        transmission,
        transmission_color,
        transmission_depth,
        transmission_extra_roughness,
        thin_walled,
        opacity,
        subsurface,
        subsurface_color,
        coat,
        coat_color,
        coat_IOR,
        // Extra params
        directDiffuse,
        directSpecular,
        specularFresnel,
        FresnelAffectDiffuse,
        FresnelUseIOR,
        Ksn,
        specular, // Ks,
        base, // Kd,
        specular_IOR, //IOR,
        IrradianceEnv,
        SpecularEnv,
        worldNormal,
        worldView,
        result);

    // Compute emission
    result.color += standardShaderEmission(emission_color, emission);
}
