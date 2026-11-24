fn mx_directional_light(light: LightData, position: vec3f, result: ptr<function, lightshader>) {
    (*result).direction = -light.direction;
    (*result).intensity = light.color * light.intensity;
}
