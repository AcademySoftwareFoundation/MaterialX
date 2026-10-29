fn mx_point_light(light: LightData, position: vec3f, result: ptr<function, lightshader>) {
    (*result).direction = light.position - position;
    let distance = length((*result).direction) + M_FLOAT_EPS;
    let attenuation = pow(distance + 1.0, light.decay_rate + M_FLOAT_EPS);
    (*result).intensity = light.color * light.intensity / attenuation;
    (*result).direction = (*result).direction / distance;
}
