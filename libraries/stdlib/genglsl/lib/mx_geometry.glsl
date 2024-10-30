
// blend 3 normals by blending the gradients
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
