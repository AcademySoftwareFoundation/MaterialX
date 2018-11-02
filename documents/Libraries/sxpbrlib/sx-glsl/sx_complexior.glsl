#include "sxpbrlib/sx-glsl/lib/sx_refractionindex.glsl"

void sx_complexior(vec3 ior, vec3 extinction, out vec3 reflectivity, out vec3 edgecolor)
{
    sx_complex_to_artistic_ior(ior, extinction, reflectivity, edgecolor);
}
