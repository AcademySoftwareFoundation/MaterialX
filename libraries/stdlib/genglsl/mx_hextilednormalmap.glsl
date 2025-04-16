#include "lib/$fileTransformUv"
#include "lib/mx_hextile.glsl"
#include "lib/mx_geometry.glsl"

// Morten S. Mikkelsen, Practical Real-Time Hex-Tiling, Journal of Computer Graphics
// Techniques (JCGT), vol. 11, no. 2, 77-94, 2022
// http://jcgt.org/published/0011/03/05/
void mx_hextilednormalmap_vector3(
    sampler2D tex_sampler,
    vec3 default_value,
    vec2 tex_coord,
    vec2 tiling,
    float rotation,
    vec2 rotation_range,
    float scale,
    vec2 scale_range,
    float offset,
    vec2 offset_range,
    float falloff,
    float strength,
    bool flip_g,
    vec3 N,
    vec3 T,
    vec3 B,
    out vec3 result
)
{
    vec2 coord = mx_transform_uv(tex_coord, tiling, vec2(0.0));

    HextileData tile_data = mx_hextile_coord(coord, rotation, rotation_range, scale, scale_range, offset, offset_range);

    vec3 nm1 = textureGrad(tex_sampler, tile_data.coord1, tile_data.ddx1, tile_data.ddy1).xyz;
    vec3 nm2 = textureGrad(tex_sampler, tile_data.coord2, tile_data.ddx2, tile_data.ddy2).xyz;
    vec3 nm3 = textureGrad(tex_sampler, tile_data.coord3, tile_data.ddx3, tile_data.ddy3).xyz;
    nm1.y = flip_g ? 1.0 - nm1.y : nm1.y;
    nm2.y = flip_g ? 1.0 - nm2.y : nm2.y;
    nm3.y = flip_g ? 1.0 - nm3.y : nm3.y;

    // normalmap to shading normal
    nm1 = 2.0 * nm1 - 1.0;
    nm2 = 2.0 * nm2 - 1.0;
    nm3 = 2.0 * nm3 - 1.0;
    mat3 tangent_rot_mat1 = mx_axis_rotation_matrix(N, -tile_data.rot_radian1);
    mat3 tangent_rot_mat2 = mx_axis_rotation_matrix(N, -tile_data.rot_radian2);
    mat3 tangent_rot_mat3 = mx_axis_rotation_matrix(N, -tile_data.rot_radian3);
    vec3 T1 = tangent_rot_mat1 * T * strength;
    vec3 T2 = tangent_rot_mat2 * T * strength;
    vec3 T3 = tangent_rot_mat3 * T * strength;
    vec3 B1 = tangent_rot_mat1 * B * strength;
    vec3 B2 = tangent_rot_mat2 * B * strength;
    vec3 B3 = tangent_rot_mat3 * B * strength;
    vec3 N1 = normalize(T1 * nm1.x + B1 * nm1.y + N * nm1.z);
    vec3 N2 = normalize(T2 * nm2.x + B2 * nm2.y + N * nm2.z);
    vec3 N3 = normalize(T3 * nm3.x + B3 * nm3.y + N * nm3.z);

    // blend weights
    vec3 w = pow(tile_data.weights, vec3(7.0));
    w /= (w.x + w.y + w.z);

    // apply s-curve gain
    if (falloff != 0.5)
    {
        w.x = mx_schlick_gain(w.x, falloff);
        w.y = mx_schlick_gain(w.y, falloff);
        w.z = mx_schlick_gain(w.z, falloff);
        w /= (w.x + w.y + w.z);
    }

    // blend
    result = mx_gradient_blend_3_normals(N, N1, w.x, N2, w.y, N3, w.z);
}
