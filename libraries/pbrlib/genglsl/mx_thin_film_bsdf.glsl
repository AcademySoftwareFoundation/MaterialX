void mx_thin_film_bsdf_reflection(vec3 L, vec3 V, vec3 P, float occlusion, float thickness, float ior, inout BSDF bsdf)
{
    bsdf.tf_thickness = thickness;
    bsdf.tf_ior = ior;
}

void mx_thin_film_bsdf_transmission(vec3 V, float thickness, float ior, inout BSDF bsdf)
{
    bsdf.tf_thickness = thickness;
    bsdf.tf_ior = ior;
}

void mx_thin_film_bsdf_indirect(vec3 V, float thickness, float ior, inout BSDF bsdf)
{
    bsdf.tf_thickness = thickness;
    bsdf.tf_ior = ior;
}
