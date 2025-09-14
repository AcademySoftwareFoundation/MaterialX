// https://www.shadertoy.com/view/4djSRW
vec2 mx_hextile_hash(vec2 p)
{
    vec3 p3 = fract(vec3(p.x, p.y, p.x) * vec3(0.1031, 0.1030, 0.0973));
    p3 += dot(p3, vec3(p3.y, p3.z, p3.x) + 33.33);
    return fract((vec2(p3.x, p3.x) + vec2(p3.y, p3.z)) * vec2(p3.z, p3.y));
}

// Christophe Schlick. “Fast Alternatives to Perlin’s Bias and Gain Functions”.
// In Graphics Gems IV, Morgan Kaufmann, 1994, pages 401–403.
// https://dept-info.labri.fr/~schlick/DOC/gem2.html
float mx_schlick_gain(float x, float r)
{
    float rr = clamp(r, 0.001, 0.999);  // to avoid glitch
    float a = (1.0 / rr - 2.0) * (1.0 - 2.0 * x);
    return (x < 0.5) ? x / (a + 1.0) : (a - x) / (a - 1.0);
}

struct HextileData
{
    vec2 coord1;
    vec2 coord2;
    vec2 coord3;
    vec3 weights;
    float rot_radian1;
    float rot_radian2;
    float rot_radian3;
    vec2 ddx1;
    vec2 ddx2;
    vec2 ddx3;
    vec2 ddy1;
    vec2 ddy2;
    vec2 ddy3;
};

// Morten S. Mikkelsen, Practical Real-Time Hex-Tiling, Journal of Computer Graphics
// Techniques (JCGT), vol. 11, no. 2, 77-94, 2022
// http://jcgt.org/published/0011/03/05/
HextileData mx_hextile_coord(
    vec2 coord,
    float rotation,
    vec2 rotation_range,
    float scale,
    vec2 scale_range,
    float offset,
    vec2 offset_range)
{
    float sqrt3_2 = sqrt(3.0) * 2.0;

    // scale coord to maintain the original fit
    vec2 st = coord * sqrt3_2;

    // skew input space into simplex triangle grid
    // (1, 0, -tan(30), 2*tan(30))
    mat2 to_skewed = mat2(1.0, 0.0, -0.57735027, 1.15470054);
    vec2 st_skewed = to_skewed * st;

    // barycentric weights
    vec2 st_frac = fract(st_skewed);
    vec3 temp = vec3(st_frac.x, st_frac.y, 0.0);
    temp.z = 1.0 - temp.x - temp.y;

    float s = step(0.0, -temp.z);
    float s2 = 2.0 * s - 1.0;

    float w1 = -temp.z * s2;
    float w2 = s - temp.y * s2;
    float w3 = s - temp.x * s2;

    // vertex IDs
    ivec2 base_id = ivec2(floor(st_skewed));
    int si = int(s);
    ivec2 id1 = base_id + ivec2(si, si);
    ivec2 id2 = base_id + ivec2(si, 1 - si);
    ivec2 id3 = base_id + ivec2(1 - si, si);

    // tile center
    mat2 inv_skewed = mat2(1.0, 0.0, 0.5, 1.0 / 1.15470054);
    vec2 ctr1 = inv_skewed * vec2(id1) / vec2(sqrt3_2);
    vec2 ctr2 = inv_skewed * vec2(id2) / vec2(sqrt3_2);
    vec2 ctr3 = inv_skewed * vec2(id3) / vec2(sqrt3_2);

    // reuse hash for performance
    vec2 seed_offset = vec2(0.12345);  // to avoid some zeros
    vec2 rand1 = mx_hextile_hash(vec2(id1) + seed_offset);
    vec2 rand2 = mx_hextile_hash(vec2(id2) + seed_offset);
    vec2 rand3 = mx_hextile_hash(vec2(id3) + seed_offset);

    // randomized rotation matrix
    vec2 rr = mx_radians(rotation_range);
    float rv1 = mix(rr.x, rr.y, rand1.x * rotation);
    float rv2 = mix(rr.x, rr.y, rand2.x * rotation);
    float rv3 = mix(rr.x, rr.y, rand3.x * rotation);
    float sin_r1 = sin(rv1);
    float sin_r2 = sin(rv2);
    float sin_r3 = sin(rv3);
    float cos_r1 = cos(rv1);
    float cos_r2 = cos(rv2);
    float cos_r3 = cos(rv3);
    mat2 rm1 = mat2(cos_r1, -sin_r1, sin_r1, cos_r1);
    mat2 rm2 = mat2(cos_r2, -sin_r2, sin_r2, cos_r2);
    mat2 rm3 = mat2(cos_r3, -sin_r3, sin_r3, cos_r3);

    // randomized scale
    vec2 sr = scale_range;
    vec2 scale1 = vec2(mix(1.0, mix(sr.x, sr.y, rand1.y), scale));
    vec2 scale2 = vec2(mix(1.0, mix(sr.x, sr.y, rand2.y), scale));
    vec2 scale3 = vec2(mix(1.0, mix(sr.x, sr.y, rand3.y), scale));

    // randomized offset
    vec2 offset1 = mix(vec2(offset_range.x), vec2(offset_range.y), rand1 * offset);
    vec2 offset2 = mix(vec2(offset_range.x), vec2(offset_range.y), rand2 * offset);
    vec2 offset3 = mix(vec2(offset_range.x), vec2(offset_range.y), rand3 * offset);

    HextileData tile_data;
    tile_data.weights = vec3(w1, w2, w3);
    tile_data.rot_radian1 = rv1;
    tile_data.rot_radian2 = rv2;
    tile_data.rot_radian3 = rv3;

    // get coord
    tile_data.coord1 = ((coord - ctr1) * rm1 / scale1) + ctr1 + offset1;
    tile_data.coord2 = ((coord - ctr2) * rm2 / scale2) + ctr2 + offset2;
    tile_data.coord3 = ((coord - ctr3) * rm3 / scale3) + ctr3 + offset3;

    // derivatives
    vec2 ddx = dFdx(coord);
    vec2 ddy = dFdy(coord);
    tile_data.ddx1 = ddx * rm1 / scale1;
    tile_data.ddx2 = ddx * rm2 / scale2;
    tile_data.ddx3 = ddx * rm3 / scale3;
    tile_data.ddy1 = ddy * rm1 / scale1;
    tile_data.ddy2 = ddy * rm2 / scale2;
    tile_data.ddy3 = ddy * rm3 / scale3;

    return tile_data;
}
