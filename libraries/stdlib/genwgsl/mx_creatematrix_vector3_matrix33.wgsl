// Generated from libraries/stdlib/genglsl/mx_creatematrix_vector3_matrix33.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_creatematrix_vector3_matrix33(in1_8: vec3f, in2_8: vec3f, in3_2: vec3f, result_82: ptr<function, mat3x3f>) {
    var in1_9: vec3f;
    var in2_9: vec3f;
    var in3_3: vec3f;

    in1_9 = in1_8;
    in2_9 = in2_8;
    in3_3 = in3_2;
    (*result_82) = mat3x3f(vec3f(in1_9.x, in1_9.y, in1_9.z), vec3f(in2_9.x, in2_9.y, in2_9.z), vec3f(in3_3.x, in3_3.y, in3_3.z));
    return;
}
