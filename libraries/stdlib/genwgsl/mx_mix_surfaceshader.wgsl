// Generated from libraries/stdlib/genglsl/mx_mix_surfaceshader.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_mix_surfaceshader(fg_9: surfaceshader, bg_9: surfaceshader, w: f32, returnshader: ptr<function, surfaceshader>) {
    var fg_10: surfaceshader;
    var bg_10: surfaceshader;

    fg_10 = fg_9;
    bg_10 = bg_9;
    (*returnshader).color = mix(bg_10.color, fg_10.color, vec3(w));
    (*returnshader).transparency = mix(bg_10.transparency, fg_10.transparency, vec3(w));
    return;
}
