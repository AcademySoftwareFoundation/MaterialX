// Generated from libraries/stdlib/genglsl/mx_transformmatrix_vector2M3.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_transformmatrix_vector2M3(val_3: vec2f, transform_1: mat3x3f, result_82: ptr<function, vec2f>) {
    var val_4: vec2f;
    var transform_2: mat3x3f;
    var res: vec3f;

    val_4 = val_3;
    transform_2 = transform_1;
    let _e24 = val_4;
    res = (mx_matrix_mul_mat3_vec3(transform_2, vec3f(_e24.x, _e24.y, 1.0)));
    (*result_82) = res.xy;
    return;
}
