// Generated from libraries/stdlib/genglsl/mx_ramptb_vector3.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_ramptb_vector3(valuet_7: vec3f, valueb_7: vec3f, texcoord_29: vec2f, result_82: ptr<function, vec3f>) {
    var valuet_8: vec3f;
    var valueb_8: vec3f;
    var texcoord_30: vec2f;

    valuet_8 = valuet_7;
    valueb_8 = valueb_7;
    texcoord_30 = texcoord_29;
    (*result_82) = mix(valueb_8, valuet_8, vec3(clamp(texcoord_30.y, 0.0, 1.0)));
    return;
}
