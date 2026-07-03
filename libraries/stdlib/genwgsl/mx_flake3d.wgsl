// Generated from libraries/stdlib/genglsl/mx_flake3d.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_flake.wgsl"

fn mx_flake3d(size_2: f32, roughness_29: f32, coverage_2: f32, position_13: vec3f, normal_2: vec3f, tangent_2: vec3f, bitangent_2: vec3f, id_2: ptr<function, i32>, rand_2: ptr<function, f32>, presence_2: ptr<function, f32>, flakenormal_2: ptr<function, vec3f>) {
    var size_3: f32;
    var roughness_30: f32;
    var coverage_3: f32;
    var position_14: vec3f;
    var normal_3: vec3f;
    var tangent_3: vec3f;
    var bitangent_3: vec3f;

    size_3 = size_2;
    roughness_30 = roughness_29;
    coverage_3 = coverage_2;
    position_14 = position_13;
    normal_3 = normal_2;
    tangent_3 = tangent_2;
    bitangent_3 = bitangent_2;
    mx_flake(size_3, roughness_30, coverage_3, position_14, normal_3, tangent_3, bitangent_3, id_2, rand_2, presence_2, flakenormal_2);
    return;
}
