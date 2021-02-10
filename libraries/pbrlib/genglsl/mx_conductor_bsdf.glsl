#include "pbrlib/genglsl/lib/mx_microfacet_specular.glsl"

void mx_conductor_bsdf_reflection(vec3 L, vec3 V, vec3 P, float occlusion, float weight, vec3 ior_n, vec3 ior_k, vec2 roughness, vec3 N, vec3 X, int distribution, thinfilm tf, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    N = mx_forward_facing_normal(N, V);

    vec3 Y = normalize(cross(N, X));
    vec3 H = normalize(L + V);

    float NdotL = clamp(dot(N, L), M_FLOAT_EPS, 1.0);
    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);
    float NdotH = clamp(dot(N, H), M_FLOAT_EPS, 1.0);
    float VdotH = clamp(dot(V, H), M_FLOAT_EPS, 1.0);

    FresnelData fd = tf.thickness > 0.0 ? mx_init_fresnel_conductor_airy(ior_n, ior_k, tf.thickness, tf.ior) : mx_init_fresnel_conductor(ior_n, ior_k);
    vec3 F = mx_compute_fresnel(VdotH, fd);

    float avgRoughness = mx_average_roughness(roughness);
    float D = mx_ggx_NDF(X, Y, H, NdotH, roughness.x, roughness.y);
    float G = mx_ggx_smith_G(NdotL, NdotV, avgRoughness);

    vec3 comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);

    // Note: NdotL is cancelled out
    result = D * F * G * comp * occlusion * weight / (4 * NdotV);
}

void mx_conductor_bsdf_indirect(vec3 V, float weight, vec3 ior_n, vec3 ior_k, vec2 roughness, vec3 N, vec3 X, int distribution, thinfilm tf, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = BSDF(0.0);
        return;
    }

    N = mx_forward_facing_normal(N, V);

    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    FresnelData fd = tf.thickness > 0.0 ? mx_init_fresnel_conductor_airy(ior_n, ior_k, tf.thickness, tf.ior) : mx_init_fresnel_conductor(ior_n, ior_k);
    vec3 F = mx_compute_fresnel(NdotV, fd);

    vec3 Li = mx_environment_radiance(N, V, X, roughness, distribution, fd);

    float avgRoughness = mx_average_roughness(roughness);
    vec3 comp = mx_ggx_energy_compensation(NdotV, avgRoughness, F);

    result = Li * comp * weight;
}
