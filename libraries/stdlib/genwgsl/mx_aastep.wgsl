// Generated from libraries/stdlib/genglsl/mx_aastep.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_aastep(threshold: f32, value_2: f32) -> f32 {
    var value_3: f32;
    var afwidth: f32;

    value_3 = value_2;
    afwidth = (length(vec2f(dpdx(value_3), dpdy(value_3))) * 0.70710677);
    return smoothstep((threshold - afwidth), (threshold + afwidth), value_3);
}
