// Generated from libraries/stdlib/genglsl/mx_surface_unlit.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_surface_unlit(emission: f32, emission_color: vec3f, transmission: f32, transmission_color: vec3f, opacity: f32, result_82: ptr<function, surfaceshader>) {

    (*result_82).color = ((emission * emission_color) * opacity);
    (*result_82).transparency = mix(vec3(1.0), (transmission * transmission_color), vec3(opacity));
    return;
}
