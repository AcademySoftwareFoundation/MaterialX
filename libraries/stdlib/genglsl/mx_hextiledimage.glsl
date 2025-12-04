#include "lib/$fileTransformUv"
#include "lib/mx_hextile.glsl"

// Morten S. Mikkelsen, Practical Real-Time Hex-Tiling, Journal of Computer Graphics
// Techniques (JCGT), vol. 11, no. 2, 77-94, 2022
// http://jcgt.org/published/0011/03/05/
void mx_hextiledimage_color3(
    $texSamplerSignature,
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

    vec3 c1 = textureGrad($texSamplerSampler2D, tile_data.coords[0], tile_data.ddx[0], tile_data.ddy[0]).rgb;
    vec3 c2 = textureGrad($texSamplerSampler2D, tile_data.coords[1], tile_data.ddx[1], tile_data.ddy[1]).rgb;
    vec3 c3 = textureGrad($texSamplerSampler2D, tile_data.coords[2], tile_data.ddx[2], tile_data.ddy[2]).rgb;

    // luminance as weights
    vec3 cw = vec3(dot(c1, lumacoeffs), dot(c2, lumacoeffs), dot(c3, lumacoeffs));
    cw = mix(vec3(1.0), cw, vec3(falloff_contrast));

    vec3 w = mx_hextile_compute_blend_weights(cw, tile_data.weights, falloff);

    // blend
    result = w.x * c1 + w.y * c2 + w.z * c3;
}

void mx_hextiledimage_color4(
    $texSamplerSignature,
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

    vec4 c1 = textureGrad($texSamplerSampler2D, tile_data.coords[0], tile_data.ddx[0], tile_data.ddy[0]);
    vec4 c2 = textureGrad($texSamplerSampler2D, tile_data.coords[1], tile_data.ddx[1], tile_data.ddy[1]);
    vec4 c3 = textureGrad($texSamplerSampler2D, tile_data.coords[2], tile_data.ddx[2], tile_data.ddy[2]);

    // luminance as weights
    vec3 cw = vec3(dot(c1.rgb, lumacoeffs), dot(c2.rgb, lumacoeffs), dot(c3.rgb, lumacoeffs));
    cw = mix(vec3(1.0), cw, vec3(falloff_contrast));

    vec3 w = mx_hextile_compute_blend_weights(cw, tile_data.weights, falloff);

    // alpha
    float a = (c1.a + c2.a + c3.a) / 3.0;
    if (falloff != 0.5)
    {
        a = mx_schlick_gain(a, falloff);
    }

    // blend
    result.rgb = w.x * c1.rgb + w.y * c2.rgb + w.z * c3.rgb;
    result.a = a;
}
