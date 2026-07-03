// Generated from libraries/stdlib/genglsl/mx_premult_color4.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_premult_color4(_in_9: vec4f, result_82: ptr<function, vec4f>) {
    var _in_10: vec4f;

    _in_10 = _in_9;
    let _e25 = (_in_10.xyz * _in_10.w);
    (*result_82) = vec4f(_e25.x, _e25.y, _e25.z, _in_10.w);
    return;
}
