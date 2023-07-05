#include "mx_microfacet_specular.glsl"

vec3 mx_pre_convolve_environment()
{
    vec2 uv = gl_FragCoord.xy * pow(2.0, (float)$convolutionMipLevel) / vec2(2048.0, 1024.0);
    if ($convolutionMipLevel == 0) {
        return textureLod($envRadiance, uv, 0).rgb;
    }

    // Do an inverse projection, i.e. go from equiangular coordinates to cartesian coordinates
    vec3 viewDirWS = mx_latlong_map_lookup_inverse(uv);

    vec3 sanityCheck = mx_latlong_map_lookup(viewDirWS, $envMatrix, 0, $envRadiance);
    return sanityCheck;
}
