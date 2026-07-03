// Generated from libraries/pbrlib/genglsl/mx_artistic_ior.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_artistic_ior(reflectivity: vec3f, edge_color: vec3f, ior_8: ptr<function, vec3f>, extinction_1: ptr<function, vec3f>) {
    var r_4: vec3f;
    var r_sqrt: vec3f;
    var n_min: vec3f;
    var n_max: vec3f;
    var np1_: vec3f;
    var nm1_: vec3f;
    var k2_: vec3f;

    r_4 = clamp(reflectivity, vec3(0.0), vec3(0.99));
    r_sqrt = sqrt(r_4);
    n_min = ((vec3(1.0) - r_4) / (vec3(1.0) + r_4));
    n_max = ((vec3(1.0) + r_sqrt) / (vec3(1.0) - r_sqrt));
    (*ior_8) = mix(n_max, n_min, edge_color);
    np1_ = (((*ior_8)) + vec3(1.0));
    nm1_ = (((*ior_8)) - vec3(1.0));
    k2_ = ((((np1_ * np1_) * r_4) - (nm1_ * nm1_)) / (vec3(1.0) - r_4));
    k2_ = max(k2_, vec3(0.0));
    (*extinction_1) = sqrt(k2_);
    return;
}
