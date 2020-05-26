#include "pbrlib/genglsl/lib/mx_microfacet_specular.glsl"

vec2 mx_ggx_directional_albedo_generate_table()
{
    vec2 uv = gl_FragCoord.xy / $albedoTableSize;
    return mx_ggx_directional_albedo_importance_sample(uv.x, uv.y, vec3(1, 0, 0), vec3(0, 1, 0)).xy;
}
