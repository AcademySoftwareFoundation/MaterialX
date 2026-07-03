// Matrix-by-vector multiply used by the hardware geometric nodes (normal/tangent/
// bitangent/transform/view-direction). WGSL has no function overloading, and every
// emitter uses the mat4x4 * vec4 form, so a single definition suffices.
fn mx_matrix_mul(m: mat4x4f, v: vec4f) -> vec4f {
    return m * v;
}

// Type-suffixed forms of GLSL's overloaded mx_matrix_mul, used by the transpiled library nodes
// (transformmatrix, blackbody, ...). vector*matrix and matrix*vector follow GLSL's argument order.
fn mx_matrix_mul_vec2_mat2(v: vec2f, m: mat2x2f) -> vec2f { return v * m; }
fn mx_matrix_mul_vec3_mat3(v: vec3f, m: mat3x3f) -> vec3f { return v * m; }
fn mx_matrix_mul_vec4_mat4(v: vec4f, m: mat4x4f) -> vec4f { return v * m; }
fn mx_matrix_mul_mat2_vec2(m: mat2x2f, v: vec2f) -> vec2f { return m * v; }
fn mx_matrix_mul_mat3_vec3(m: mat3x3f, v: vec3f) -> vec3f { return m * v; }
fn mx_matrix_mul_mat4_vec4(m: mat4x4f, v: vec4f) -> vec4f { return m * v; }
fn mx_matrix_mul_mat2_mat2(a: mat2x2f, b: mat2x2f) -> mat2x2f { return a * b; }
fn mx_matrix_mul_mat3_mat3(a: mat3x3f, b: mat3x3f) -> mat3x3f { return a * b; }
fn mx_matrix_mul_mat4_mat4(a: mat4x4f, b: mat4x4f) -> mat4x4f { return a * b; }

// Square functions

fn mx_square_f32(x: f32) -> f32 {
    return (x * x);
}

fn mx_square_vec2(x: vec2f) -> vec2f {
    return (x * x);
}

fn mx_square_vec3(x: vec3f) -> vec3f {
    return (x * x);
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

fn mx_srgb_encode(color: vec3f) -> vec3f {
    let isAbove = (color > vec3(0.0031308f));
    let linSeg = (color * 12.92f);
    let powSeg = ((1.055f * pow(max(color, vec3(0f)), vec3(0.41666666f))) - vec3(0.055f));
    return select(linSeg, powSeg, isAbove);
}
