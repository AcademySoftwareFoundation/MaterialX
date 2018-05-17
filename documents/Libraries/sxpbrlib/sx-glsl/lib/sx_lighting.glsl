vec2 sx_latlong_projection(vec3 dir)
{
    float latitude = -asin(dir.y) * M_PI_INV + 0.5;
    latitude = clamp(latitude, 0.01, 0.99);
    float longitude = atan(dir.x, -dir.z) * M_PI_INV * 0.5 + 0.5;
    return vec2(longitude, latitude);
}

vec3 sx_latlong_map_lookup(vec3 dir, mat4 transform, sampler2D sampler)
{
    vec2 res = textureSize(sampler, 0);
    if (res.x > 0)
    {
        vec3 dir = normalize((transform * vec4(dir,0.0)).xyz);
        vec2 uv = sx_latlong_projection(dir);
        return texture(sampler, uv).rgb;
    }
    return vec3(0.0);
}

vec3 sx_latlong_map_lookup(vec3 dir, mat4 transform, float lodBias, sampler2D sampler)
{
    vec2 res = textureSize(sampler, 0);
    if (res.x > 0)
    {
        // Heuristic for faking a blur by roughness
        int levels = 1 + int(floor(log2(max(res.x, res.y))));
        lodBias = lodBias < 0.25 ? sqrt(lodBias) : 0.5*lodBias + 0.375;
        float lod = lodBias * levels;

        vec3 dir = normalize((transform * vec4(dir,0.0)).xyz);
        vec2 uv = sx_latlong_projection(dir);
        return textureLod(sampler, uv, lod).rgb;
    }
    return vec3(0.0);
}

vec3 sx_environment_specular(vec3 normal, vec3 view, float roughness)
{
    vec3 dir = reflect(-view, normal);
    return sx_latlong_map_lookup(dir, u_envMatrix, roughness, u_envSpecular);
}

vec3 sx_environment_irradiance(vec3 normal)
{
    vec3 result = sx_latlong_map_lookup(normal, u_envMatrix, u_envIrradiance);

    // TODO: remove when we have HDR irradiance maps
    result = pow(result, vec3(2.2));

    return result;
}
