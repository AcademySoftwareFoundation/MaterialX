fn mx_spot_light(light: LightData, position: vec3f, result: ptr<function, lightshader>) {
    (*result).direction = light.position - position;
    let distance = length((*result).direction) + M_FLOAT_EPS;
    let attenuation = pow(distance + 1.0, light.decay_rate + M_FLOAT_EPS);
    (*result).intensity = light.color * light.intensity / attenuation;
    (*result).direction = (*result).direction / distance;
    let low = min(light.inner_angle, light.outer_angle);
    let high = light.inner_angle;
    let cosDir = dot((*result).direction, -light.direction);
    let spotAttenuation = smoothstep(low, high, cosDir);
    (*result).intensity = (*result).intensity * spotAttenuation;
}
