void mx_spotlight(LightData light, vec3 position, out lightshader result)
{
    result.direction = light.position - position;
    float distance = length(result.direction) + M_FLOAT_EPS;
    float attenuation = pow(distance + 1.0, light.decayRate + M_FLOAT_EPS);
    result.intensity = light.color * light.intensity / attenuation;
    result.direction /= distance;
    float low = min(light.innerConeAngle, light.outerConeAngle);
    float high = light.innerConeAngle;
    float cosDir = dot(result.direction, -light.direction);
    float spotAttenuation = smoothstep(low, high, cosDir);
    result.intensity *= spotAttenuation;
}
