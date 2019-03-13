#include "pbrlib/genglsl/lib/mx_bsdfs.glsl"

void mx_sheen_brdf_reflection(vec3 L, vec3 V, float weight, vec3 color, float roughness, vec3 normal, BSDF base, out BSDF result)
{
    // TODO: implement this
    result = base;
}

void mx_sheen_brdf_indirect(vec3 V, float weight, vec3 color, float roughness, vec3 normal, BSDF base, out vec3 result)
{
    // TODO: implement this
    result = base;
}
