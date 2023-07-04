#include "mx_microfacet_specular.glsl"

vec3 mx_pre_convolve_environment()
{
    vec2 uv = gl_FragCoord.xy / vec2(2048.0, 1024.0);
    // vec2 ggxDirAlbedo = mx_ggx_dir_albedo(uv.x, uv.y, vec3(1, 0, 0), vec3(0, 1, 0)).xy;
    return textureLod($envRadiance, uv, 0).rgb * vec3(uv, 0.0);
    // return vec3(uv, 0.0);
}
