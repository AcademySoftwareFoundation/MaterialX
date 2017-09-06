#include "sx/impl/shadergen/source/glsl/shadingmodel_functions.glsl"

void sx_reflectionbsdf(vec3 reflectance, float roughness, vec3 normal, int distribution, out vec4 result)
{
    vec3 worldNormal = normalize(normal);
    vec3 worldView = normalize(PS_IN.WorldView);

    vec3 specularInput = vec3(0.0);
    computeSpecular(roughness, worldNormal, worldView, specularInput);

    vec3 specularEnv = EnvironmentLight(worldNormal, worldView, roughness);

    result = vec4(reflectance * (specularInput + specularEnv), 1.0);
}
