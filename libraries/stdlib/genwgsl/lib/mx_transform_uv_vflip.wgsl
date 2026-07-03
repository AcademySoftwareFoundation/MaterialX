// UV transform with vertical flip: scale, offset, then flip V.
fn mx_transform_uv(uv: vec2f, uv_scale: vec2f, uv_offset: vec2f) -> vec2f {
    let transformed = uv * uv_scale + uv_offset;
    return vec2f(transformed.x, 1.0 - transformed.y);
}
