void sx_pointlight(LightData light, vec3 position, out lightshader result)
{
    result.direction = light.position - position;
    float distance = length(result.direction);
    float attenuation = pow(distance, light.decayRate);
    result.intensity = light.color * light.intensity / attenuation;
    result.direction /= distance;
}
