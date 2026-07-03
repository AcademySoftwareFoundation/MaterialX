// Generated from libraries/stdlib/genglsl/mx_rgbtohsv_color3.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_hsv.wgsl"

fn mx_rgbtohsv_color3(_in_9: vec3f, result_82: ptr<function, vec3f>) {
    var _in_10: vec3f;

    _in_10 = _in_9;
    (*result_82) = mx_rgbtohsv(_in_10);
    return;
}
