// Generated from libraries/stdlib/genglsl/mx_rotate_vector2.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_rotate_vector2(_in_9: vec2f, amount_1: f32, result_82: ptr<function, vec2f>) {
    var _in_10: vec2f;
    var amount_2: f32;
    var rotationRadians: f32;
    var sa: f32;
    var ca: f32;

    _in_10 = _in_9;
    amount_2 = amount_1;
    rotationRadians = radians(amount_2);
    sa = sin(rotationRadians);
    ca = cos(rotationRadians);
    (*result_82) = vec2f(((ca * _in_10.x) + (sa * _in_10.y)), ((-(sa) * _in_10.x) + (ca * _in_10.y)));
    return;
}
