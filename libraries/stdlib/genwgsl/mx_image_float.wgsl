#include "lib/$fileTransformUv"

fn mx_image_float(tex: texture_2d<f32>, samp: sampler, layer: i32, defaultval: f32, texcoord: vec2f, uaddressmode: i32, vaddressmode: i32, filtertype: i32, framerange: i32, frameoffset: i32, frameendaction: i32, uv_scale: vec2f, uv_offset: vec2f, result: ptr<function, f32>) {
    let uv = mx_transform_uv(texcoord, uv_scale, uv_offset);
    *result = textureSample(tex, samp, uv).r;
}
