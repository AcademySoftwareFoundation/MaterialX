#include "pbrlib/genglsl/lib/mx_refraction_index.glsl"

void mx_complex_ior(vec3 ior, vec3 extinction, out vec3 reflectivity, out vec3 edge_color)
{
    mx_complex_to_artistic_ior(ior, extinction, reflectivity, edge_color);
}
