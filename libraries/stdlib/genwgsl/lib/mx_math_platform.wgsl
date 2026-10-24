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

// Matrix inverse — WGSL has no built-in inverse(); ported from genmsl/lib/mx_math.metal.

fn mx_inverse_mat3(m: mat3x3f) -> mat3x3f {
    let d = determinant(m);
    let invdet = select(1.0 / d, 0.0, d == 0.0);
    return mat3x3f(
        vec3f((m[1][1] * m[2][2] - m[2][1] * m[1][2]) * invdet,
              (m[2][1] * m[0][2] - m[0][1] * m[2][2]) * invdet,
              (m[0][1] * m[1][2] - m[1][1] * m[0][2]) * invdet),
        vec3f((m[2][0] * m[1][2] - m[1][0] * m[2][2]) * invdet,
              (m[0][0] * m[2][2] - m[2][0] * m[0][2]) * invdet,
              (m[1][0] * m[0][2] - m[0][0] * m[1][2]) * invdet),
        vec3f((m[1][0] * m[2][1] - m[2][0] * m[1][1]) * invdet,
              (m[2][0] * m[0][1] - m[0][0] * m[2][1]) * invdet,
              (m[0][0] * m[1][1] - m[1][0] * m[0][1]) * invdet)
    );
}

fn mx_inverse_mat4(m: mat4x4f) -> mat4x4f {
    let n11 = m[0][0]; let n12 = m[1][0]; let n13 = m[2][0]; let n14 = m[3][0];
    let n21 = m[0][1]; let n22 = m[1][1]; let n23 = m[2][1]; let n24 = m[3][1];
    let n31 = m[0][2]; let n32 = m[1][2]; let n33 = m[2][2]; let n34 = m[3][2];
    let n41 = m[0][3]; let n42 = m[1][3]; let n43 = m[2][3]; let n44 = m[3][3];

    let t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
    let t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
    let t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
    let t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

    let d = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
    let invdet = select(1.0 / d, 0.0, d == 0.0);

    return mat4x4f(
        vec4f(t11 * invdet,
              (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * invdet,
              (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * invdet,
              (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * invdet),
        vec4f(t12 * invdet,
              (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * invdet,
              (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * invdet,
              (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * invdet),
        vec4f(t13 * invdet,
              (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * invdet,
              (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * invdet,
              (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * invdet),
        vec4f(t14 * invdet,
              (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * invdet,
              (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * invdet,
              (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * invdet)
    );
}

// Matrix conditional — WGSL select() does not support matrix types.

fn mx_ifgreater_mat3(v1: f32, v2: f32, a: mat3x3f, b: mat3x3f) -> mat3x3f {
    if (v1 > v2) { return a; } else { return b; }
}

fn mx_ifgreater_mat4(v1: f32, v2: f32, a: mat4x4f, b: mat4x4f) -> mat4x4f {
    if (v1 > v2) { return a; } else { return b; }
}

fn mx_ifgreatereq_mat3(v1: f32, v2: f32, a: mat3x3f, b: mat3x3f) -> mat3x3f {
    if (v1 >= v2) { return a; } else { return b; }
}

fn mx_ifgreatereq_mat4(v1: f32, v2: f32, a: mat4x4f, b: mat4x4f) -> mat4x4f {
    if (v1 >= v2) { return a; } else { return b; }
}

fn mx_ifequal_mat3_float(v1: f32, v2: f32, a: mat3x3f, b: mat3x3f) -> mat3x3f {
    if (v1 == v2) { return a; } else { return b; }
}

fn mx_ifequal_mat4_float(v1: f32, v2: f32, a: mat4x4f, b: mat4x4f) -> mat4x4f {
    if (v1 == v2) { return a; } else { return b; }
}

fn mx_ifequal_mat3_int(v1: i32, v2: i32, a: mat3x3f, b: mat3x3f) -> mat3x3f {
    if (v1 == v2) { return a; } else { return b; }
}

fn mx_ifequal_mat4_int(v1: i32, v2: i32, a: mat4x4f, b: mat4x4f) -> mat4x4f {
    if (v1 == v2) { return a; } else { return b; }
}

fn mx_ifequal_mat3_bool(v1: bool, v2: bool, a: mat3x3f, b: mat3x3f) -> mat3x3f {
    if (v1 == v2) { return a; } else { return b; }
}

fn mx_ifequal_mat4_bool(v1: bool, v2: bool, a: mat4x4f, b: mat4x4f) -> mat4x4f {
    if (v1 == v2) { return a; } else { return b; }
}

fn mx_ifgreater_i_mat3(v1: i32, v2: i32, a: mat3x3f, b: mat3x3f) -> mat3x3f {
    if (v1 > v2) { return a; } else { return b; }
}

fn mx_ifgreater_i_mat4(v1: i32, v2: i32, a: mat4x4f, b: mat4x4f) -> mat4x4f {
    if (v1 > v2) { return a; } else { return b; }
}

fn mx_ifgreatereq_i_mat3(v1: i32, v2: i32, a: mat3x3f, b: mat3x3f) -> mat3x3f {
    if (v1 >= v2) { return a; } else { return b; }
}

fn mx_ifgreatereq_i_mat4(v1: i32, v2: i32, a: mat4x4f, b: mat4x4f) -> mat4x4f {
    if (v1 >= v2) { return a; } else { return b; }
}

// Stub for transpiled textureSize(..., 0).x in directional-albedo table branches (mxgenwgsl).
fn mtlx_tex_size_x() -> f32 {
    return 256.0;
}

fn mx_add_mat3x3(a: mat3x3f, b: mat3x3f) -> mat3x3f {
    return a + b;
}

fn mx_sub_mat3x3(a: mat3x3f, b: mat3x3f) -> mat3x3f {
    return a - b;
}

fn mx_add_mat4x4(a: mat4x4f, b: mat4x4f) -> mat4x4f {
    return a + b;
}

fn mx_sub_mat4x4(a: mat4x4f, b: mat4x4f) -> mat4x4f {
    return a - b;
}

fn mx_add_mat3x3_f32(m: mat3x3f, s: f32) -> mat3x3f {
    let v = vec3(s);
    return mat3x3f(m[0] + v, m[1] + v, m[2] + v);
}

fn mx_add_mat4x4_f32(m: mat4x4f, s: f32) -> mat4x4f {
    let v = vec4(s);
    return mat4x4f(m[0] + v, m[1] + v, m[2] + v, m[3] + v);
}

fn mx_sub_mat3x3_f32(m: mat3x3f, s: f32) -> mat3x3f {
    let v = vec3(s);
    return mat3x3f(m[0] - v, m[1] - v, m[2] - v);
}

fn mx_sub_mat4x4_f32(m: mat4x4f, s: f32) -> mat4x4f {
    let v = vec4(s);
    return mat4x4f(m[0] - v, m[1] - v, m[2] - v, m[3] - v);
}
