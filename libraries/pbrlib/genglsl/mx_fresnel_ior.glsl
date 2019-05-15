#include "pbrlib/genglsl/lib/mx_bsdfs.glsl"

void mx_fresnel_ior(float ior, vec3 N, vec3 V, out float result)
{
    float cosTheta = dot(N, -V);
    result = mx_fresnel_dielectric(cosTheta, ior);
}
