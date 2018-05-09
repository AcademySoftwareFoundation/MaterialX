const vec2 invAtan = vec2(0.1591, 0.3183);

vec3 sx_latlong_map_lookup(vec3 dir, float lodBias, sampler2D sampler)
{
    vec2 res = textureSize(sampler, 0);
    if (res.x > 0)
    {
        vec2 uv = vec2(atan(-dir.x, dir.z), asin(-dir.y)) * invAtan + 0.5;
        int levels = 1 + int(floor(log2(max(res.x, res.y))));
        lodBias = lodBias < 0.25 ? sqrt(lodBias) : 0.5*lodBias + 0.375;
        float lod = lodBias * levels;
        return textureLod(sampler, uv, lod).rgb;
    }
    return vec3(0.0);
}

vec3 sx_environment_specular(vec3 normal, vec3 view, float roughness)
{
    vec3 dir = reflect(-view, normal);
    return sx_latlong_map_lookup(dir, roughness, u_envSpecular);
}

vec3 sx_environment_irradiance(vec3 normal)
{
    return sx_latlong_map_lookup(normal, 1.0, u_envIrradiance);
}
