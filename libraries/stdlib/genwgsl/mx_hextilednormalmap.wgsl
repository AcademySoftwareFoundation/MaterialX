#include "lib/$fileTransformUv"
#include "lib/mx_hextile.wgsl"
#include "lib/mx_geometry.wgsl"

//
// Morten S. Mikkelsen, Practical Real-Time Hex-Tiling, JCGT vol. 11, no. 2, 2022
// http://jcgt.org/published/0011/03/05/

fn mx_hextilednormalmap_vector3(
    tex: texture_2d<f32>,
    tex_sampler: sampler,
    default_value: vec3f,
    tex_coord: vec2f,
    tiling: vec2f,
    rotation: f32,
    rotation_range: vec2f,
    scale: f32,
    scale_range: vec2f,
    offset_val: f32,
    offset_range: vec2f,
    falloff: f32,
    strength: f32,
    flip_g: bool,
    N: vec3f,
    T: vec3f,
    B: vec3f,
    result: ptr<function, vec3f>
) {
    let coord = mx_transform_uv(tex_coord, tiling, vec2f(0.0));

    let tile_data = mx_hextile_coord(coord, rotation, rotation_range, scale, scale_range, offset_val, offset_range);

    var nm1 = textureSampleGrad(tex, tex_sampler, tile_data.coords0, tile_data.ddx0, tile_data.ddy0).xyz;
    var nm2 = textureSampleGrad(tex, tex_sampler, tile_data.coords1, tile_data.ddx1, tile_data.ddy1).xyz;
    var nm3 = textureSampleGrad(tex, tex_sampler, tile_data.coords2, tile_data.ddx2, tile_data.ddy2).xyz;

    if (flip_g) {
        nm1.y = 1.0 - nm1.y;
        nm2.y = 1.0 - nm2.y;
        nm3.y = 1.0 - nm3.y;
    }

    // Normal map to shading normal.
    nm1 = 2.0 * nm1 - 1.0;
    nm2 = 2.0 * nm2 - 1.0;
    nm3 = 2.0 * nm3 - 1.0;
    let tangent_rot_mat1 = mx_axis_rotation_matrix(N, -tile_data.rotations.x);
    let tangent_rot_mat2 = mx_axis_rotation_matrix(N, -tile_data.rotations.y);
    let tangent_rot_mat3 = mx_axis_rotation_matrix(N, -tile_data.rotations.z);
    let T1 = (tangent_rot_mat1 * T) * strength;
    let T2 = (tangent_rot_mat2 * T) * strength;
    let T3 = (tangent_rot_mat3 * T) * strength;
    let B1 = (tangent_rot_mat1 * B) * strength;
    let B2 = (tangent_rot_mat2 * B) * strength;
    let B3 = (tangent_rot_mat3 * B) * strength;
    let N1 = normalize(T1 * nm1.x + B1 * nm1.y + N * nm1.z);
    let N2 = normalize(T2 * nm2.x + B2 * nm2.y + N * nm2.z);
    let N3 = normalize(T3 * nm3.x + B3 * nm3.y + N * nm3.z);

    // Blend weights.
    let w = mx_hextile_compute_blend_weights(vec3f(1.0), tile_data.weights, falloff);

    *result = mx_gradient_blend_3_normals(N, N1, w.x, N2, w.y, N3, w.z);
}
