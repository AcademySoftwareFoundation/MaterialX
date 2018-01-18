#include "stdlib/impl/shadergen/source/glsl/aastep.glsl"

void mx_splitlr_float(float valuel, float valuer, float center, vec2 texcoord, out float result)
{
    result = mix(valuel, valuer, aastep (center, texcoord.x));
}
