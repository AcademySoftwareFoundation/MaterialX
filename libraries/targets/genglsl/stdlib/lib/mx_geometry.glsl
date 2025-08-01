// Blend 3 normals by blending the gradients
// Morten S. Mikkelsen, Surface Gradient–Based Bump Mapping Framework, Journal of
// Computer Graphics Techniques (JCGT), vol. 9, no. 3, 60–90, 2020
// http://jcgt.org/published/0009/03/04/
vec3 mx_normals_to_gradient(vec3 N, vec3 Np)
{
    float d = dot(N, Np);
    vec3 g = (d * N - Np) / max(M_FLOAT_EPS, abs(d));
    return g;
}

vec3 mx_gradient_blend_3_normals(vec3 N, vec3 N1, float N1_weight, vec3 N2, float N2_weight, vec3 N3, float N3_weight)
{
    float w1 = clamp(N1_weight, 0.0, 1.0);
    float w2 = clamp(N2_weight, 0.0, 1.0);
    float w3 = clamp(N3_weight, 0.0, 1.0);

    vec3 g1 = mx_normals_to_gradient(N, N1);
    vec3 g2 = mx_normals_to_gradient(N, N2);
    vec3 g3 = mx_normals_to_gradient(N, N3);

    // blend
    vec3 gg = w1 * g1 + w2 * g2 + w3 * g3;

    // gradient to normal
    return normalize(N - gg);
}

// This function should be categorized in mx_math.glsl but it causes build errors in MSL
// so adding here for a workaround
mat3 mx_axis_rotation_matrix(vec3 a, float r)
{
    float s = sin(r);
    float c = cos(r);
    float omc = 1.0 - c;
    return mat3(
        a.x*a.x*omc + c,     a.x*a.y*omc - a.z*s, a.x*a.z*omc + a.y*s,
        a.y*a.x*omc + a.z*s, a.y*a.y*omc + c,     a.y*a.z*omc - a.x*s,
        a.z*a.x*omc - a.y*s, a.z*a.y*omc + a.x*s, a.z*a.z*omc + c
    );
}
