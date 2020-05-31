vec3 mx_latlong_map_lookup(vec3 dir, mat4 transform, sampler2D sampler)
{
    vec2 res = textureSize(sampler, 0);
    if (res.x > 0)
    {
        vec3 dir = normalize((transform * vec4(dir,0.0)).xyz);
        vec2 uv = mx_latlong_projection(dir);
        return texture(sampler, uv).rgb;
    }
    return vec3(0.0);
}

vec3 mx_latlong_map_lookup(vec3 dir, mat4 transform, float lodBias, sampler2D sampler)
{
    vec2 res = textureSize(sampler, 0);
    if (res.x > 0)
    {
        // Heuristic for faking a blur by roughness
        int levels = 1 + int(floor(log2(max(res.x, res.y))));
        lodBias = lodBias < 0.25 ? sqrt(lodBias) : 0.5*lodBias + 0.375;
        float lod = lodBias * levels;

        vec3 dir = normalize((transform * vec4(dir,0.0)).xyz);
        vec2 uv = mx_latlong_projection(dir);
        return textureLod(sampler, uv, lod).rgb;
    }
    return vec3(0.0);
}

vec3 mx_environment_radiance(vec3 N, vec3 V, vec3 X, vec2 roughness, vec3 F0, vec3 F90, vec3 iorN, vec3 iorK, int distribution, int fresnelModel)
{
    vec3 L = reflect(-V, N);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    float avgRoughness = mx_average_roughness(roughness);

    vec3 dirAlbedo = mx_ggx_directional_albedo(NdotV, avgRoughness, F0, F90);
    return mx_latlong_map_lookup(L, $envMatrix, avgRoughness, $envRadiance) * dirAlbedo;
}

vec3 mx_environment_irradiance(vec3 N)
{
    return mx_latlong_map_lookup(N, $envMatrix, $envIrradiance);
}
