// Generated from libraries/stdlib/genglsl/mx_splitlr_float.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "mx_aastep.wgsl"

fn mx_splitlr_float(valuel_7: f32, valuer_7: f32, center_7: f32, texcoord_29: vec2f, result_82: ptr<function, f32>) {
    var valuel_8: f32;
    var valuer_8: f32;
    var center_8: f32;
    var texcoord_30: vec2f;

    valuel_8 = valuel_7;
    valuer_8 = valuer_7;
    center_8 = center_7;
    texcoord_30 = texcoord_29;
    (*result_82) = mix(valuel_8, valuer_8, (mx_aastep(center_8, texcoord_30.x)));
    return;
}
