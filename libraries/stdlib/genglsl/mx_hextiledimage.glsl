#include "lib/$fileTransformUv"
#include "lib/mx_hextile.glsl"

// Morten S. Mikkelsen, Practical Real-Time Hex-Tiling, Journal of Computer Graphics
// Techniques (JCGT), vol. 11, no. 2, 77-94, 2022
// http://jcgt.org/published/0011/03/05/
void mx_hextiledimage_color3(
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
    float falloff_contrast,
    vec3 lumacoeffs,
    out vec3 result
)
{
    vec2 coord = mx_transform_uv(tex_coord, tiling, vec2(0.0));

    HextileData tile_data = mx_hextile_coord(coord, rotation, rotation_range, scale, scale_range, offset, offset_range);

    vec3 c1 = textureGrad(tex_sampler, tile_data.coord1, tile_data.ddx1, tile_data.ddy1).rgb;
    vec3 c2 = textureGrad(tex_sampler, tile_data.coord2, tile_data.ddx2, tile_data.ddy2).rgb;
    vec3 c3 = textureGrad(tex_sampler, tile_data.coord3, tile_data.ddx3, tile_data.ddy3).rgb;

    // luminance as weights
    vec3 cw = vec3(dot(c1, lumacoeffs), dot(c2, lumacoeffs), dot(c3, lumacoeffs));
    cw = mix(vec3(1.0), cw, vec3(falloff_contrast));

    // blend weights
    vec3 w = cw * pow(tile_data.weights, vec3(7.0));
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
    result = vec3(w.x * c1 + w.y * c2 + w.z * c3);
}

void mx_hextiledimage_color4(
    sampler2D tex_sampler,
    vec4 default_value,
    vec2 tex_coord,
    vec2 tiling,
    float rotation,
    vec2 rotation_range,
    float scale,
    vec2 scale_range,
    float offset,
    vec2 offset_range,
    float falloff,
    float falloff_contrast,
    vec3 lumacoeffs,
    out vec4 result
)
{
    vec2 coord = mx_transform_uv(tex_coord, tiling, vec2(0.0));

    HextileData tile_data = mx_hextile_coord(coord, rotation, rotation_range, scale, scale_range, offset, offset_range);

    vec4 c1 = textureGrad(tex_sampler, tile_data.coord1, tile_data.ddx1, tile_data.ddy1);
    vec4 c2 = textureGrad(tex_sampler, tile_data.coord2, tile_data.ddx2, tile_data.ddy2);
    vec4 c3 = textureGrad(tex_sampler, tile_data.coord3, tile_data.ddx3, tile_data.ddy3);

    // luminance as weights
    vec3 cw = vec3(dot(c1.rgb, lumacoeffs), dot(c2.rgb, lumacoeffs), dot(c3.rgb, lumacoeffs));
    cw = mix(vec3(1.0), cw, vec3(falloff_contrast));

    // blend weights
    vec3 w = cw * pow(tile_data.weights, vec3(7.0));
    w /= (w.x + w.y + w.z);

    // alpha
    float a = (c1.a + c2.a + c3.a) / 3.0;

    // apply s-curve gain
    if (falloff != 0.5)
    {
        w.x = mx_schlick_gain(w.x, falloff);
        w.y = mx_schlick_gain(w.y, falloff);
        w.z = mx_schlick_gain(w.z, falloff);
        w /= (w.x + w.y + w.z);
        a = mx_schlick_gain(a, falloff);
    }

    // blend
    result.rgb = vec3(w.x * c1 + w.y * c2 + w.z * c3);
    result.a = a;
}
