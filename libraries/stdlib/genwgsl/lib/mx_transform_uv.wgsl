// UV transform: scale and offset.
fn mx_transform_uv(uv: vec2f, uv_scale: vec2f, uv_offset: vec2f) -> vec2f {
    return uv * uv_scale + uv_offset;
}
