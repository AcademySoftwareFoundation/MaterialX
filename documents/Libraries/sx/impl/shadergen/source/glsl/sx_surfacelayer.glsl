void sx_surfacelayer_scattering(BSDF bsdf, vec3 opacity, surfaceshader baselayer, out surfaceshader result)
{
    result.bsdf.fr = bsdf.fr * opacity + baselayer.bsdf.fr * (1 - opacity);
    result.opacity = max(opacity, baselayer.opacity); 
}

void sx_surfacelayer_emission(vec3 edf, vec3 opacity, surfaceshader baselayer, out surfaceshader result)
{
    result.edf = edf * opacity + baselayer.edf * (1 - opacity);
    result.opacity = max(opacity, baselayer.opacity); 
}
