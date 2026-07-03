// Generated from libraries/stdlib/genglsl/mx_smoothstep_float.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_smoothstep_float(val_3: f32, low: f32, high: f32, result_82: ptr<function, f32>) {
    var val_4: f32;

    val_4 = val_3;
    if (val_4 >= high) {
        (*result_82) = 1.0;
        return;
    } else {
        if (val_4 <= low) {
            (*result_82) = 0.0;
            return;
        } else {
            (*result_82) = smoothstep(low, high, val_4);
            return;
        }
    }
}
