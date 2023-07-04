#include "mx_microfacet_specular.glsl"

vec3 mx_pre_convolve_environment()
{
    vec2 uv = gl_FragCoord.xy * pow(2.0, (float)$convolutionMipLevel) / vec2(2048.0, 1024.0);
    // vec2 ggxDirAlbedo = mx_ggx_dir_albedo(uv.x, uv.y, vec3(1, 0, 0), vec3(0, 1, 0)).xy;
    vec3 col = vec3(0, 0, 0);
    switch ($convolutionMipLevel)
    {
        case 0:
            col = vec3(1.0, 0.0, 0.0);
            break;
        case 1:
            col = vec3(0.0, 1.0, 0.0);
            break;
        case 2:
            col = vec3(0.0, 0.0, 1.0);
            break;
        case 3:
            col = vec3(1.0, 0.0, 1.0);
            break;
        case 4:
            col = vec3(1.0, 1.0, 0.0);
            break;
        case 5:
            col = vec3(0.0, 1.0, 1.0);
            break;
        case 6:
            col = vec3(1.0, 1.0, 1.0);
            break;
    }
    return textureLod($envRadiance, uv, 0).rgb * col;
    // return vec3(uv, 0.0);
}
