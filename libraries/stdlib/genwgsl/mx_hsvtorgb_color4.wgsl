// Generated from libraries/stdlib/genglsl/mx_hsvtorgb_color4.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_hsv.wgsl"

fn mx_hsvtorgb_color4(_in_9: vec4f, result_82: ptr<function, vec4f>) {
    var _in_10: vec4f;

    _in_10 = _in_9;
    let _e23 = mx_hsvtorgb(_in_10.xyz);
    (*result_82) = vec4f(_e23.x, _e23.y, _e23.z, _in_10.w);
    return;
}
