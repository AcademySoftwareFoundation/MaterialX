#include "lib/$fileTransformUv"
#include "lib/mx_hextile.wgsl"

//
// Morten S. Mikkelsen, Practical Real-Time Hex-Tiling, JCGT vol. 11, no. 2, 2022
// http://jcgt.org/published/0011/03/05/

fn mx_hextiledimage_color3(
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
    falloff_contrast: f32,
    lumacoeffs: vec3f,
    result: ptr<function, vec3f>
) {
    let coord = mx_transform_uv(tex_coord, tiling, vec2f(0.0));

    let tile_data = mx_hextile_coord(coord, rotation, rotation_range, scale, scale_range, offset_val, offset_range);

    let c1 = textureSampleGrad(tex, tex_sampler, tile_data.coords0, tile_data.ddx0, tile_data.ddy0).rgb;
    let c2 = textureSampleGrad(tex, tex_sampler, tile_data.coords1, tile_data.ddx1, tile_data.ddy1).rgb;
    let c3 = textureSampleGrad(tex, tex_sampler, tile_data.coords2, tile_data.ddx2, tile_data.ddy2).rgb;

    // Luminance as weights.
    var cw = vec3f(dot(c1, lumacoeffs), dot(c2, lumacoeffs), dot(c3, lumacoeffs));
    cw = mix(vec3f(1.0), cw, vec3f(falloff_contrast));

    let w = mx_hextile_compute_blend_weights(cw, tile_data.weights, falloff);

    *result = w.x * c1 + w.y * c2 + w.z * c3;
}

fn mx_hextiledimage_color4(
    tex: texture_2d<f32>,
    tex_sampler: sampler,
    default_value: vec4f,
    tex_coord: vec2f,
    tiling: vec2f,
    rotation: f32,
    rotation_range: vec2f,
    scale: f32,
    scale_range: vec2f,
    offset_val: f32,
    offset_range: vec2f,
    falloff: f32,
    falloff_contrast: f32,
    lumacoeffs: vec3f,
    result: ptr<function, vec4f>
) {
    let coord = mx_transform_uv(tex_coord, tiling, vec2f(0.0));

    let tile_data = mx_hextile_coord(coord, rotation, rotation_range, scale, scale_range, offset_val, offset_range);

    let c1 = textureSampleGrad(tex, tex_sampler, tile_data.coords0, tile_data.ddx0, tile_data.ddy0);
    let c2 = textureSampleGrad(tex, tex_sampler, tile_data.coords1, tile_data.ddx1, tile_data.ddy1);
    let c3 = textureSampleGrad(tex, tex_sampler, tile_data.coords2, tile_data.ddx2, tile_data.ddy2);

    // Luminance as weights.
    var cw = vec3f(dot(c1.rgb, lumacoeffs), dot(c2.rgb, lumacoeffs), dot(c3.rgb, lumacoeffs));
    cw = mix(vec3f(1.0), cw, vec3f(falloff_contrast));

    let w = mx_hextile_compute_blend_weights(cw, tile_data.weights, falloff);

    // Alpha is averaged, then optionally gain-adjusted.
    var a = (c1.a + c2.a + c3.a) / 3.0;
    if (falloff != 0.5) {
        a = mx_schlick_gain(a, falloff);
    }

    (*result) = vec4f(w.x * c1.rgb + w.y * c2.rgb + w.z * c3.rgb, a);
}
