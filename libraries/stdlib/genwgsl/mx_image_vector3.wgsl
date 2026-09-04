#include "lib/$fileTransformUv"

fn mx_image_vector3(tex: texture_2d<f32>, samp: sampler, layer: i32, defaultval: vec3f, texcoord: vec2f, uaddressmode: i32, vaddressmode: i32, filtertype: i32, framerange: i32, frameoffset: i32, frameendaction: i32, uv_scale: vec2f, uv_offset: vec2f, result: ptr<function, vec3f>) {
    let uv = mx_transform_uv(texcoord, uv_scale, uv_offset);
    *result = textureSample(tex, samp, uv).rgb;
}
