#include "lib/mx_flake.glsl"

void mx_flake2d(
    float size,
    float roughness,
    float coverage,
    vec2 texcoord,
    vec3 normal,
    vec3 tangent,
    vec3 bitangent,
    out int id,
    out float rand,
    out float presence,
    out vec3 flakenormal
)
{
    // reuse the 3d flake implementation. this could be optimized by using a 2d flake implementation
    vec3 position = vec3(texcoord.x, texcoord.y, 0.0);
    mx_flake(size, roughness, coverage, position, normal, tangent, bitangent, id, rand, presence, flakenormal);
}