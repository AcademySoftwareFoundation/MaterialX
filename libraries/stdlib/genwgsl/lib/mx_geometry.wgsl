// Normal blending and axis rotation utilities.
// Port of genglsl/lib/mx_geometry.glsl

// Blend 3 normals by blending the gradients.
// Morten S. Mikkelsen, Surface Gradient–Based Bump Mapping Framework, JCGT vol. 9, no. 3, 2020
// http://jcgt.org/published/0009/03/04/
fn mx_normals_to_gradient(N: vec3f, Np: vec3f) -> vec3f {
    let d = dot(N, Np);
    return (d * N - Np) / max(M_FLOAT_EPS, abs(d));
}

fn mx_gradient_blend_3_normals(N: vec3f, N1: vec3f, N1_weight: f32, N2: vec3f, N2_weight: f32, N3: vec3f, N3_weight: f32) -> vec3f {
    let w1 = clamp(N1_weight, 0.0, 1.0);
    let w2 = clamp(N2_weight, 0.0, 1.0);
    let w3 = clamp(N3_weight, 0.0, 1.0);

    let g1 = mx_normals_to_gradient(N, N1);
    let g2 = mx_normals_to_gradient(N, N2);
    let g3 = mx_normals_to_gradient(N, N3);

    let gg = w1 * g1 + w2 * g2 + w3 * g3;
    return normalize(N - gg);
}

// Rotation matrix around an arbitrary axis.
// (Placed here rather than mx_math.wgsl to match MaterialX's workaround for MSL builds.)
fn mx_axis_rotation_matrix(a: vec3f, r: f32) -> mat3x3f {
    let s = sin(r);
    let c = cos(r);
    let omc = 1.0 - c;
    return mat3x3f(
        a.x * a.x * omc + c,       a.x * a.y * omc - a.z * s, a.x * a.z * omc + a.y * s,
        a.y * a.x * omc + a.z * s, a.y * a.y * omc + c,       a.y * a.z * omc - a.x * s,
        a.z * a.x * omc - a.y * s, a.z * a.y * omc + a.x * s, a.z * a.z * omc + c
    );
}
