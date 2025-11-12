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
    vec2 coords[3];
    vec3 weights;
    vec3 rotations;
    vec2 ddx[3];
    vec2 ddy[3];
};

// Helper function to compute blend weights with optional falloff
vec3 mx_hextile_compute_blend_weights(vec3 luminance_weights, vec3 tile_weights, float falloff)
{
    vec3 w = luminance_weights * pow(tile_weights, vec3(7.0));
    w /= (w.x + w.y + w.z);

    if (falloff != 0.5)
    {
        w.x = mx_schlick_gain(w.x, falloff);
        w.y = mx_schlick_gain(w.y, falloff);
        w.z = mx_schlick_gain(w.z, falloff);
        w /= (w.x + w.y + w.z);
    }
    return w;
}

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
    vec2 st_skewed = mx_matrix_mul(to_skewed, st);

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
    vec2 ctr1 = mx_matrix_mul(inv_skewed, vec2(id1) / vec2(sqrt3_2));
    vec2 ctr2 = mx_matrix_mul(inv_skewed, vec2(id2) / vec2(sqrt3_2));
    vec2 ctr3 = mx_matrix_mul(inv_skewed, vec2(id3) / vec2(sqrt3_2));

    // reuse hash for performance
    vec2 seed_offset = vec2(0.12345);  // to avoid some zeros
    vec2 rand1 = mx_hextile_hash(vec2(id1) + seed_offset);
    vec2 rand2 = mx_hextile_hash(vec2(id2) + seed_offset);
    vec2 rand3 = mx_hextile_hash(vec2(id3) + seed_offset);

    // randomized rotation matrix
    vec2 rr = mx_radians(rotation_range);
    vec3 rand_x = vec3(rand1.x, rand2.x, rand3.x);
    vec3 rotations = mix(vec3(rr.x), vec3(rr.y), rand_x * rotation);
    vec3 sin_r = sin(rotations);
    vec3 cos_r = cos(rotations);
    mat2 rm1 = mat2(cos_r.x, -sin_r.x, sin_r.x, cos_r.x);
    mat2 rm2 = mat2(cos_r.y, -sin_r.y, sin_r.y, cos_r.y);
    mat2 rm3 = mat2(cos_r.z, -sin_r.z, sin_r.z, cos_r.z);

    // randomized scale
    vec3 rand_y = vec3(rand1.y, rand2.y, rand3.y);
    vec3 scales = mix(vec3(1.0), mix(vec3(scale_range.x), vec3(scale_range.y), rand_y), scale);
    vec2 scale1 = vec2(scales.x);
    vec2 scale2 = vec2(scales.y);
    vec2 scale3 = vec2(scales.z);

    // randomized offset
    vec2 offset1 = mix(vec2(offset_range.x), vec2(offset_range.y), rand1 * offset);
    vec2 offset2 = mix(vec2(offset_range.x), vec2(offset_range.y), rand2 * offset);
    vec2 offset3 = mix(vec2(offset_range.x), vec2(offset_range.y), rand3 * offset);

    HextileData tile_data;
    tile_data.weights = vec3(w1, w2, w3);
    tile_data.rotations = rotations;

    // get coord
    tile_data.coords[0] = (mx_matrix_mul((coord - ctr1), rm1) / scale1) + ctr1 + offset1;
    tile_data.coords[1] = (mx_matrix_mul((coord - ctr2), rm2) / scale2) + ctr2 + offset2;
    tile_data.coords[2] = (mx_matrix_mul((coord - ctr3), rm3) / scale3) + ctr3 + offset3;

    // derivatives
    vec2 ddx = dFdx(coord);
    vec2 ddy = dFdy(coord);
    tile_data.ddx[0] = mx_matrix_mul(ddx, rm1) / scale1;
    tile_data.ddx[1] = mx_matrix_mul(ddx, rm2) / scale2;
    tile_data.ddx[2] = mx_matrix_mul(ddx, rm3) / scale3;
    tile_data.ddy[0] = mx_matrix_mul(ddy, rm1) / scale1;
    tile_data.ddy[1] = mx_matrix_mul(ddy, rm2) / scale2;
    tile_data.ddy[2] = mx_matrix_mul(ddy, rm3) / scale3;

    return tile_data;
}
