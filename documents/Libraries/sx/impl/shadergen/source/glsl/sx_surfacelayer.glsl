void sx_surfacelayer(vec3 influence, vec4 bsdf, vec3 edf, float ior, surfaceshader baselayer, out surfaceshader result)
{
    vec3 w = saturate(influence);
    if (ior > 0.0)
    {
        vec3 normal = normalize(PS_IN.WorldNormal);
        vec3 view = normalize(PS_IN.WorldView);
        w *= FresnelSchlickTIR(ior, 1.0, normal, view);
//        w *= FresnelSchlickTIR(ior, baselayer.ior, normal, view);
    }

    result.bsdf.rgb = baselayer.bsdf.rgb * (1.0 - w) + bsdf.rgb * w;

    // TODO: what about alpha?
    result.bsdf.a = baselayer.bsdf.a;

    result.edf  = baselayer.edf  * (1.0 - w) + edf;
    result.ior  = ior;
}
