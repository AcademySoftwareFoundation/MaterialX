// Generated from libraries/pbrlib/genglsl/mx_roughness_anisotropy.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

fn mx_roughness_anisotropy(roughness_29: f32, anisotropy_2: f32, result_82: ptr<function, vec2f>) {
    var roughness_30: f32;
    var anisotropy_3: f32;
    var roughness_sqr: f32;
    var aspect: f32;

    roughness_30 = roughness_29;
    anisotropy_3 = anisotropy_2;
    roughness_sqr = clamp((roughness_30 * roughness_30), 0.00000001, 1.0);
    if (anisotropy_3 > 0.0) {
        {
            aspect = sqrt((1.0 - clamp(anisotropy_3, 0.0, 0.98)));
            (*result_82).x = min((roughness_sqr / aspect), 1.0);
            (*result_82).y = (roughness_sqr * aspect);
            return;
        }
    } else {
        {
            (*result_82).x = roughness_sqr;
            (*result_82).y = roughness_sqr;
            return;
        }
    }
}
