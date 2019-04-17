#include "pbrlib/genglsl/lib/mx_bsdfs.glsl"

void mx_sheen_brdf_reflection(vec3 L, vec3 V, float weight, vec3 color, float roughness, vec3 N, BSDF base, out BSDF result)
{
    if (weight < M_FLOAT_EPS)
    {
        result = base;
        return;
    }

    float NdotL = dot(N,L);
    float NdotV = dot(N,V);
    if (NdotL <= 0.0 || NdotV <= 0.0)
    {
        result = base;
        return;
    }

    vec3 H = normalize(L + V);
    float NdotH = dot(N, H);

    float alpha = clamp(roughness, M_FLOAT_EPS, 1.0);
    float D = mx_microfacet_sheen_NDF(NdotH, alpha);

    vec3 F = color * weight;

    // Geometry term is skipped and we use a smoother denominator, as in:
    // https://blog.selfshadow.com/publications/s2013-shading-course/rad/s2013_pbs_rad_notes.pdf
    vec3 fr = D * F / (4.0 * (NdotL + NdotV - NdotL*NdotV));

    // Top layer transmission to atttenuate base layer.
    // We use a LUT of directional albedo for this.
    float albedo = mx_microfacet_sheen_albedo(NdotV, alpha);
    float topTrans = clamp(1.0 - albedo, 0.0, 1.0);

    // We need to include NdotL from the light integral here
    // as in this case it's not cancelled out by the BRDF denominator.
    result = fr * NdotL        // Top layer reflection
           + base * topTrans;  // Base layer reflection attenuated by top layer
}

void mx_sheen_brdf_transmission(vec3 V, float weight, vec3 color, float roughness, vec3 N, BSDF base, out BSDF result)
{
    // Ignore sheen for transparent base layers
    result = base;
}

void mx_sheen_brdf_indirect(vec3 V, float weight, vec3 color, float roughness, vec3 N, BSDF base, out vec3 result)
{
    // TODO: Implement indirect sheen
    result = base;
}
