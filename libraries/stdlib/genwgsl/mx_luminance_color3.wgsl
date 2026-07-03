// Generated from libraries/stdlib/genglsl/mx_luminance_color3.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_luminance_color3(_in_9: vec3f, lumacoeffs_1: vec3f, result_82: ptr<function, vec3f>) {
    var _in_10: vec3f;
    var lumacoeffs_2: vec3f;

    _in_10 = _in_9;
    lumacoeffs_2 = lumacoeffs_1;
    (*result_82) = vec3(dot(_in_10, lumacoeffs_2));
    return;
}
