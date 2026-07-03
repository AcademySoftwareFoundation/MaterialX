// Generated from libraries/stdlib/genglsl/mx_dodge_color3.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "mx_dodge_float.wgsl"

fn mx_dodge_color3(fg_9: vec3f, bg_9: vec3f, mixval_6: f32, result_82: ptr<function, vec3f>) {
    var fg_10: vec3f;
    var bg_10: vec3f;
    var mixval_7: f32;
    var f_1: f32;

    fg_10 = fg_9;
    bg_10 = bg_9;
    mixval_7 = mixval_6;
    mx_dodge_float(fg_10.x, bg_10.x, mixval_7, (&f_1));
    (*result_82).x = f_1;
    mx_dodge_float(fg_10.y, bg_10.y, mixval_7, (&f_1));
    (*result_82).y = f_1;
    mx_dodge_float(fg_10.z, bg_10.z, mixval_7, (&f_1));
    (*result_82).z = f_1;
    return;
}
