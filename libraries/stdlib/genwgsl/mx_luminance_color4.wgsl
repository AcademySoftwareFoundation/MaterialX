// Generated from libraries/stdlib/genglsl/mx_luminance_color4.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_luminance_color4(_in_9: vec4f, lumacoeffs_1: vec3f, result_82: ptr<function, vec4f>) {
    var _in_10: vec4f;
    var lumacoeffs_2: vec3f;

    _in_10 = _in_9;
    lumacoeffs_2 = lumacoeffs_1;
    let _e27 = vec3(dot(_in_10.xyz, lumacoeffs_2));
    (*result_82) = vec4f(_e27.x, _e27.y, _e27.z, _in_10.w);
    return;
}
