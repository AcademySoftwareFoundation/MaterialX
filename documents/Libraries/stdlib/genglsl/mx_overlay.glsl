float mx_overlay(float fg, float bg)
{
    return (fg < 0.5) ? (2 * fg * bg) : (1 - (1 - fg) * (1 - bg));
}

vec2 mx_overlay(vec2 fg, vec2 bg)
{
    return vec2(mx_overlay(fg.r, bg.r),
                mx_overlay(fg.g, bg.g));
}

vec3 mx_overlay(vec3 fg, vec3 bg)
{
    return vec3(mx_overlay(fg.r, bg.r),
                mx_overlay(fg.g, bg.g),
                mx_overlay(fg.b, bg.b));
}

vec4 mx_overlay(vec4 fg, vec4 bg)
{
    return vec4(mx_overlay(fg.r, bg.r),
                mx_overlay(fg.g, bg.g),
                mx_overlay(fg.b, bg.b),
                mx_overlay(fg.a, bg.a));
}
