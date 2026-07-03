// Generated from libraries/pbrlib/genglsl/mx_displacement_vector3.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_displacement_vector3(disp_1: vec3f, scale_3: f32, result_82: ptr<function, displacementshader>) {
    var disp_2: vec3f;
    var scale_4: f32;

    disp_2 = disp_1;
    scale_4 = scale_3;
    (*result_82).offset = disp_2;
    (*result_82).scale = scale_4;
    return;
}
