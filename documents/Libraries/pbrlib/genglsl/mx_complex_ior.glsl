#include "pbrlib/genglsl/lib/mx_refractionindex.glsl"

void mx_complex_ior(vec3 ior, vec3 extinction, out vec3 reflectivity, out vec3 edgecolor)
{
    mx_complex_to_artistic_ior(ior, extinction, reflectivity, edgecolor);
}
