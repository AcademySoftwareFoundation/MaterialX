#include "lib/mx_flake.glsl"

void mx_flake3d(
    float size,
    float roughness,
    float coverage,
    vec3 position,
    vec3 normal,
    vec3 tangent,
    vec3 bitangent,
    out int id,
    out float rand,
    out float presence,
    out vec3 flakenormal
)
{
    mx_flake(size, roughness, coverage, position, normal, tangent, bitangent, id, rand, presence, flakenormal);
}