// Generated from libraries/stdlib/genglsl/mx_splitlr_vector2.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "mx_aastep.wgsl"

fn mx_splitlr_vector2(valuel_7: vec2f, valuer_7: vec2f, center_7: f32, texcoord_29: vec2f, result_82: ptr<function, vec2f>) {
    var valuel_8: vec2f;
    var valuer_8: vec2f;
    var center_8: f32;
    var texcoord_30: vec2f;

    valuel_8 = valuel_7;
    valuer_8 = valuer_7;
    center_8 = center_7;
    texcoord_30 = texcoord_29;
    (*result_82) = mix(valuel_8, valuer_8, vec2((mx_aastep(center_8, texcoord_30.x))));
    return;
}
