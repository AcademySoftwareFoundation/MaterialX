// Generated from libraries/stdlib/genglsl/mx_disjointover_color4.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_disjointover_color4(fg_9: vec4f, bg_9: vec4f, mixval_6: f32, result_82: ptr<function, vec4f>) {
    var fg_10: vec4f;
    var bg_10: vec4f;
    var mixval_7: f32;
    var summedAlpha: f32;
    var x_34: f32;

    fg_10 = fg_9;
    bg_10 = bg_9;
    mixval_7 = mixval_6;
    summedAlpha = (fg_10.w + bg_10.w);
    if (summedAlpha <= 1.0) {
        {
            let _e34 = (*result_82);
            let _e40 = (fg_10.xyz + bg_10.xyz);
            (*result_82).x = _e40.x;
            (*result_82).y = _e40.y;
            (*result_82).z = _e40.z;
        }
    } else {
        {
            if (abs(bg_10.w) < 0.00000001) {
                {
                    let _e52 = (*result_82);
                    (*result_82).x = 0.0;
                    (*result_82).y = 0.0;
                    (*result_82).z = 0.0;
                }
            } else {
                {
                    x_34 = ((1.0 - fg_10.w) / bg_10.w);
                    let _e67 = (*result_82);
                    let _e75 = (fg_10.xyz + (bg_10.xyz * x_34));
                    (*result_82).x = _e75.x;
                    (*result_82).y = _e75.y;
                    (*result_82).z = _e75.z;
                }
            }
        }
    }
    (*result_82).w = min(summedAlpha, 1.0);
    let _e86 = (*result_82);
    let _e98 = ((((*result_82)).xyz * mixval_7) + ((1.0 - mixval_7) * bg_10.xyz));
    (*result_82).x = _e98.x;
    (*result_82).y = _e98.y;
    (*result_82).z = _e98.z;
    (*result_82).w = ((((*result_82)).w * mixval_7) + ((1.0 - mixval_7) * bg_10.w));
    return;
}
