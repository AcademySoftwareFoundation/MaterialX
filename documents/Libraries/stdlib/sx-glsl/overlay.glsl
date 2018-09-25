float overlay(float fg, float bg)
{
    return (fg < 0.5) ? (2 * fg * bg) : (1 - (1 - fg) * (1 - bg));
}

vec2 overlay(vec2 fg, vec2 bg)
{
    return vec2(overlay(fg.r, bg.r),
                overlay(fg.g, bg.g));
}

vec3 overlay(vec3 fg, vec3 bg)
{
    return vec3(overlay(fg.r, bg.r),
                overlay(fg.g, bg.g),
                overlay(fg.b, bg.b));
}

vec4 overlay(vec4 fg, vec4 bg)
{
    return vec4(overlay(fg.r, bg.r),
                overlay(fg.g, bg.g),
                overlay(fg.b, bg.b),
                overlay(fg.a, bg.a));
}
