// Environment lighting with no IBL contribution.
// WGSL equivalent of genglsl/lib/mx_environment_none.glsl.

fn mx_environment_radiance(N: vec3f, V: vec3f, X: vec3f, roughness: vec2f, distribution: i32, fd: FresnelData) -> vec3f
{
    return vec3f(0.0);
}

fn mx_environment_irradiance(N: vec3f) -> vec3f
{
    return vec3f(0.0);
}
