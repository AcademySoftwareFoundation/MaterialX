// Generated from libraries/stdlib/genglsl/mx_rotate_vector3.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_rotate_vector3(_in_9: vec3f, amount_1: f32, axis: vec3f, result_82: ptr<function, vec3f>) {
    var _in_10: vec3f;
    var amount_2: f32;
    var axis_1: vec3f;
    var rotationRadians: f32;
    var s_8: f32;
    var c_3: f32;
    var oc: f32;

    _in_10 = _in_9;
    amount_2 = amount_1;
    axis_1 = axis;
    axis_1 = normalize(axis_1);
    rotationRadians = radians(amount_2);
    s_8 = sin(rotationRadians);
    c_3 = cos(rotationRadians);
    oc = (1.0 - c_3);
    (*result_82) = (((_in_10 * c_3) + (cross(_in_10, axis_1) * s_8)) + ((axis_1 * dot(axis_1, _in_10)) * oc));
    return;
}
