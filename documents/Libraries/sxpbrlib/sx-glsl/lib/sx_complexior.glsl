// "Artist Friendly Metallic Fresnel", Ole Gulbrandsen, 2014
// http://jcgt.org/published/0003/04/03/paper.pdf

vec3 sx_get_reflectivity(vec3 n, vec3 k)
{
    vec3 nm1 = n - 1;
    vec3 np1 = n + 1;
    vec3 k2  = k * k;
    return (nm1*nm1 + k2) / (np1*np1 + k2);
}

vec3 sx_get_edgetint(vec3 n, vec3 reflectivity)
{
    vec3 r = reflectivity;
    vec3 r_sqrt = sqrt(r);
    vec3 n_min = (1.0 - r) / (1.0 + r);
    vec3 n_max = (1.0 + r_sqrt) / (1.0 - r_sqrt);
    return (n_max - n) / (n_max - n_min);
}

void sx_complexior(vec3 reflectivity, vec3 edgetint, out vec3 n, out vec3 k)
{
    vec3 r = clamp(reflectivity, 0.0, 0.99);
    vec3 g = edgetint;

    vec3 r_sqrt = sqrt(r);
    vec3 n_min = (1.0 - r) / (1.0 + r);
    vec3 n_max = (1.0 + r_sqrt) / (1.0 - r_sqrt);
    n = mix(n_max, n_min, g);

    vec3 k2 = (sx_square(n + 1.0) * r - sx_square(n - 1.0)) / (1.0 - r);
    k2 = max(k2, 0.0);
    k = sqrt(k2);
}
