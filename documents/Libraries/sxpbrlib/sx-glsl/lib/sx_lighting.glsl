vec2 sx_spherical_coords(vec3 vec)
{
    float v = acos(clamp(vec.z, -1.0, 1.0));
    float u = clamp(vec.x / sin(v), -1.0, 1.0);
    if (vec.y >= 0.0)
        u = acos(u);
    else
        u = 2 * M_PI - acos(u);
    return vec2(u / (2 * M_PI), v / M_PI);
}

vec3 sx_latlong_map_lookup(vec3 dir, float lodBias, sampler2D sampler)
{
    vec2 res = textureSize(sampler, 0);
    if (res.x > 0)
    {
        vec2 uv = sx_spherical_coords(dir);
        int levels = 1 + int(floor(log2(max(res.x, res.y))));
        float lod = lodBias * levels;
        return textureLod(sampler, uv, lod).rgb;
    }
    return vec3(0.0);
}

vec3 sx_environment_specular(vec3 normal, vec3 view, float roughness)
{
    vec3 dir = reflect(-view, normal);
    // Y is up vector
    dir = vec3(dir.x, -dir.z, dir.y);

    return sx_latlong_map_lookup(dir, roughness, u_envSpecular);
}

vec3 sx_environment_irradiance(vec3 normal)
{
    return sx_latlong_map_lookup(normal, 1.0, u_envIrradiance);
}
