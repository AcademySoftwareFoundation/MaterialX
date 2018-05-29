#include "sxpbrlib/sx-glsl/lib/sx_complexior.glsl"

void sx_reflectivity(vec3 ior_n, vec3 ior_k, out vec3 result)
{
    result = sx_get_reflectivity(ior_n, ior_k);
}
