// Generated from libraries/stdlib/genglsl/mx_normalmap.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_normalmap_vector2(value_2: vec3f, normal_scale_1: vec2f, N_24: vec3f, T_1: vec3f, B_1: vec3f, result_82: ptr<function, vec3f>) {
    var value_3: vec3f;
    var normal_scale_2: vec2f;
    var N_25: vec3f;
    var T_2: vec3f;
    var B_2: vec3f;
    var local: vec3f;

    value_3 = value_2;
    normal_scale_2 = normal_scale_1;
    N_25 = N_24;
    T_2 = T_1;
    B_2 = B_1;
    if (dot(value_3, value_3) == 0.0) {
        local = vec3f(0.0, 0.0, 1.0);
    } else {
        local = ((value_3 * 2.0) - vec3(1.0));
    }
    value_3 = local;
    value_3 = ((((T_2 * value_3.x) * normal_scale_2.x) + ((B_2 * value_3.y) * normal_scale_2.y)) + (N_25 * value_3.z));
    (*result_82) = normalize(value_3);
    return;
}

fn mx_normalmap_float(value_2: vec3f, normal_scale_1: f32, N_24: vec3f, T_1: vec3f, B_1: vec3f, result_82: ptr<function, vec3f>) {
    var value_3: vec3f;
    var normal_scale_2: f32;
    var N_25: vec3f;
    var T_2: vec3f;
    var B_2: vec3f;

    value_3 = value_2;
    normal_scale_2 = normal_scale_1;
    N_25 = N_24;
    T_2 = T_1;
    B_2 = B_1;
    mx_normalmap_vector2(value_3, vec2(normal_scale_2), N_25, T_2, B_2, result_82);
    return;
}
