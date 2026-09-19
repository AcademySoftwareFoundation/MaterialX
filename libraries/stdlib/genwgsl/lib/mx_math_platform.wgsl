// WGSL platform helpers that naga cannot transpile from genglsl/lib/mx_math.glsl.
// mx_mod: GLSL uses `#define mx_mod mod`; WGSL needs explicit overloads with floor semantics.
// mx_isinf: naga rejects GLSL isinf().

fn mx_isinf(v: f32) -> bool {
    // WGSL has no isInf. +/-inf is the only bit pattern with all exponent bits set and a zero
    // mantissa; masking the sign bit matches both infinities, while NaN (nonzero mantissa) does
    // not -- matching GLSL isinf(). A magnitude compare is avoided because the only correct
    // threshold is exactly FLT_MAX, and that literal overflows f32 const-eval on some drivers.
    return (bitcast<u32>(v) & 0x7fffffffu) == 0x7f800000u;
}

// Modulo with GLSL mod() semantics: x - y * floor(x / y)
// WGSL '%' operator is remainder (fmod), not modulo, so we need explicit functions.

fn mx_mod_f32(x: f32, y: f32) -> f32 {
    return x - y * floor(x / y);
}

fn mx_mod_vec2(x: vec2f, y: vec2f) -> vec2f {
    return x - y * floor(x / y);
}

fn mx_mod_vec2_f32(x: vec2f, y: f32) -> vec2f {
    return x - vec2f(y) * floor(x / vec2f(y));
}

fn mx_mod_vec3(x: vec3f, y: vec3f) -> vec3f {
    return x - y * floor(x / y);
}

fn mx_mod_vec3_f32(x: vec3f, y: f32) -> vec3f {
    return x - vec3f(y) * floor(x / vec3f(y));
}

fn mx_mod_vec4(x: vec4f, y: vec4f) -> vec4f {
    return x - y * floor(x / y);
}

fn mx_mod_vec4_f32(x: vec4f, y: f32) -> vec4f {
    return x - vec4f(y) * floor(x / vec4f(y));
}
