// Generated from libraries/pbrlib/genglsl/mx_roughness_dual.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_roughness_dual(roughness_29: vec2f, result_82: ptr<function, vec2f>) {
    var roughness_30: vec2f;

    roughness_30 = roughness_29;
    if (roughness_30.y < 0.0) {
        {
            roughness_30.y = roughness_30.x;
        }
    }
    (*result_82).x = clamp((roughness_30.x * roughness_30.x), 0.00000001, 1.0);
    (*result_82).y = clamp((roughness_30.y * roughness_30.y), 0.00000001, 1.0);
    return;
}
