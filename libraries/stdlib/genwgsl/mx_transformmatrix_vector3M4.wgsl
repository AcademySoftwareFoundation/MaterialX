// Generated from libraries/stdlib/genglsl/mx_transformmatrix_vector3M4.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_transformmatrix_vector3M4(val_3: vec3f, transform_1: mat4x4f, result_82: ptr<function, vec3f>) {
    var val_4: vec3f;
    var transform_2: mat4x4f;
    var res: vec4f;

    val_4 = val_3;
    transform_2 = transform_1;
    let _e24 = val_4;
    res = (mx_matrix_mul_mat4_vec4(transform_2, vec4f(_e24.x, _e24.y, _e24.z, 1.0)));
    (*result_82) = res.xyz;
    return;
}
