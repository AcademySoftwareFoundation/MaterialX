// Generated from libraries/stdlib/genglsl/mx_unpremult_color4.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_unpremult_color4(_in_9: vec4f, result_82: ptr<function, vec4f>) {
    var _in_10: vec4f;

    _in_10 = _in_9;
    let _e26 = (_in_10.xyz / vec3(_in_10.w));
    (*result_82) = vec4f(_e26.x, _e26.y, _e26.z, _in_10.w);
    return;
}
