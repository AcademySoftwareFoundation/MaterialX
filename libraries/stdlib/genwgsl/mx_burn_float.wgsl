// Generated from libraries/stdlib/genglsl/mx_burn_float.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_burn_float(fg_9: f32, bg_9: f32, mixval_6: f32, result_82: ptr<function, f32>) {
    var fg_10: f32;
    var bg_10: f32;
    var mixval_7: f32;

    fg_10 = fg_9;
    bg_10 = bg_9;
    mixval_7 = mixval_6;
    if (abs(fg_10) < 0.00000001) {
        {
            (*result_82) = 0.0;
            return;
        }
    }
    (*result_82) = ((mixval_7 * (1.0 - ((1.0 - bg_10) / fg_10))) + ((1.0 - mixval_7) * bg_10));
    return;
}
