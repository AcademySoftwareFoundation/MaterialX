#include "pbrlib/genglsl/lib/mx_refraction_index.glsl"

void mx_artistic_ior(vec3 reflectivity, vec3 edge_color, out vec3 ior, out vec3 extinction)
{
    mx_artistic_to_complex_ior(reflectivity, edge_color, ior, extinction);
}
