// Generated from libraries/stdlib/genglsl/mx_ramplr_vector2.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_ramplr_vector2(valuel_7: vec2f, valuer_7: vec2f, texcoord_29: vec2f, result_82: ptr<function, vec2f>) {
    var valuel_8: vec2f;
    var valuer_8: vec2f;
    var texcoord_30: vec2f;

    valuel_8 = valuel_7;
    valuer_8 = valuer_7;
    texcoord_30 = texcoord_29;
    (*result_82) = mix(valuel_8, valuer_8, vec2(clamp(texcoord_30.x, 0.0, 1.0)));
    return;
}
