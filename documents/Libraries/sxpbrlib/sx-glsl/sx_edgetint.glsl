#include "sxpbrlib/sx-glsl/lib/sx_complexior.glsl"

void sx_edgetint(vec3 ior_n, vec3 reflectivity, out vec3 result)
{
    result = sx_get_edgetint(ior_n, reflectivity);
}
