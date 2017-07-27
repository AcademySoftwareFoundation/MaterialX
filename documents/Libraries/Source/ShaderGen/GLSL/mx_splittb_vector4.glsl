#include "Source/ShaderGen/GLSL/aastep.glsl"

void mx_splittb_vector4(vec4 valuet, vec4 valueb, float center, vec2 texcoord, out vec4 result)
{
    result = mix(valuet, valueb, aastep (center, texcoord.y));
}
