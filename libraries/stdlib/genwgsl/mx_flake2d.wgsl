// Generated from libraries/stdlib/genglsl/mx_flake2d.glsl by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.
// Do not edit -- re-run the transpiler to regenerate (see source/MaterialXGenWgsl/README.md).

#include "lib/mx_flake.wgsl"

fn mx_flake2d(size_2: f32, roughness_29: f32, coverage_2: f32, texcoord_29: vec2f, normal_2: vec3f, tangent_2: vec3f, bitangent_2: vec3f, id_2: ptr<function, i32>, rand_2: ptr<function, f32>, presence_2: ptr<function, f32>, flakenormal_2: ptr<function, vec3f>) {
    var size_3: f32;
    var roughness_30: f32;
    var coverage_3: f32;
    var texcoord_30: vec2f;
    var normal_3: vec3f;
    var tangent_3: vec3f;
    var bitangent_3: vec3f;
    var position_14: vec3f;

    size_3 = size_2;
    roughness_30 = roughness_29;
    coverage_3 = coverage_2;
    texcoord_30 = texcoord_29;
    normal_3 = normal_2;
    tangent_3 = tangent_2;
    bitangent_3 = bitangent_2;
    position_14 = vec3f(texcoord_30.x, texcoord_30.y, 0.0);
    mx_flake(size_3, roughness_30, coverage_3, position_14, normal_3, tangent_3, bitangent_3, id_2, rand_2, presence_2, flakenormal_2);
    return;
}
