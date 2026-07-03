// Hex-tiling coordinate generation utilities.
//
// Morten S. Mikkelsen, Practical Real-Time Hex-Tiling, JCGT vol. 11, no. 2, 2022
// http://jcgt.org/published/0011/03/05/

// https://www.shadertoy.com/view/4djSRW
fn mx_hextile_hash(p: vec2f) -> vec2f {
    var p3 = fract(vec3f(p.x, p.y, p.x) * vec3f(0.1031, 0.1030, 0.0973));
    p3 += dot(p3, vec3f(p3.y, p3.z, p3.x) + 33.33);
    return fract((vec2f(p3.x, p3.x) + vec2f(p3.y, p3.z)) * vec2f(p3.z, p3.y));
}

// Christophe Schlick. "Fast Alternatives to Perlin's Bias and Gain Functions".
// In Graphics Gems IV, Morgan Kaufmann, 1994, pages 401–403.
// https://dept-info.labri.fr/~schlick/DOC/gem2.html
fn mx_schlick_gain(x: f32, r: f32) -> f32 {
    let rr = clamp(r, 0.001, 0.999);
    let a = (1.0 / rr - 2.0) * (1.0 - 2.0 * x);
    if (x < 0.5) {
        return x / (a + 1.0);
    }
    return (a - x) / (a - 1.0);
}

struct HextileData {
    coords0: vec2f,
    coords1: vec2f,
    coords2: vec2f,
    weights: vec3f,
    rotations: vec3f,
    ddx0: vec2f,
    ddx1: vec2f,
    ddx2: vec2f,
    ddy0: vec2f,
    ddy1: vec2f,
    ddy2: vec2f,
};

// Helper to compute blend weights with optional falloff.
fn mx_hextile_compute_blend_weights(luminance_weights: vec3f, tile_weights: vec3f, falloff: f32) -> vec3f {
    var w = luminance_weights * pow(tile_weights, vec3f(7.0));
    w /= (w.x + w.y + w.z);

    if (falloff != 0.5) {
        w.x = mx_schlick_gain(w.x, falloff);
        w.y = mx_schlick_gain(w.y, falloff);
        w.z = mx_schlick_gain(w.z, falloff);
        w /= (w.x + w.y + w.z);
    }
    return w;
}

fn mx_hextile_coord(
    coord: vec2f,
    rotation: f32,
    rotation_range: vec2f,
    scale: f32,
    scale_range: vec2f,
    offset: f32,
    offset_range: vec2f
) -> HextileData {
    let sqrt3_2 = sqrt(3.0) * 2.0;

    // Scale coord to maintain the original fit.
    let st = coord * sqrt3_2;

    // Skew input space into simplex triangle grid.
    // (1, 0, -tan(30), 2*tan(30))
    let to_skewed = mat2x2f(1.0, 0.0, -0.57735027, 1.15470054);
    let st_skewed = (to_skewed * st);

    // Barycentric weights.
    let st_frac = fract(st_skewed);
    var temp = vec3f(st_frac.x, st_frac.y, 0.0);
    temp.z = 1.0 - temp.x - temp.y;

    let s = step(0.0, -temp.z);
    let s2 = 2.0 * s - 1.0;

    let w1 = -temp.z * s2;
    let w2 = s - temp.y * s2;
    let w3 = s - temp.x * s2;

    // Vertex IDs.
    let base_id = vec2<i32>(floor(st_skewed));
    let si = i32(s);
    let id1 = base_id + vec2<i32>(si, si);
    let id2 = base_id + vec2<i32>(si, 1 - si);
    let id3 = base_id + vec2<i32>(1 - si, si);

    // Tile centers.
    let inv_skewed = mat2x2f(1.0, 0.0, 0.5, 1.0 / 1.15470054);
    let ctr1 = (inv_skewed * (vec2f(id1) / vec2f(sqrt3_2)));
    let ctr2 = (inv_skewed * (vec2f(id2) / vec2f(sqrt3_2)));
    let ctr3 = (inv_skewed * (vec2f(id3) / vec2f(sqrt3_2)));

    // Reuse hash for performance.
    let seed_offset = vec2f(0.12345);
    let rand1 = mx_hextile_hash(vec2f(id1) + seed_offset);
    let rand2 = mx_hextile_hash(vec2f(id2) + seed_offset);
    let rand3 = mx_hextile_hash(vec2f(id3) + seed_offset);

    // Randomized rotation.
    let rr = radians(rotation_range);
    let rand_x = vec3f(rand1.x, rand2.x, rand3.x);
    let rotations = mix(vec3f(rr.x), vec3f(rr.y), rand_x * rotation);
    let sin_r = sin(rotations);
    let cos_r = cos(rotations);
    let rm1 = mat2x2f(cos_r.x, -sin_r.x, sin_r.x, cos_r.x);
    let rm2 = mat2x2f(cos_r.y, -sin_r.y, sin_r.y, cos_r.y);
    let rm3 = mat2x2f(cos_r.z, -sin_r.z, sin_r.z, cos_r.z);

    // Randomized scale.
    let rand_y = vec3f(rand1.y, rand2.y, rand3.y);
    let scales = mix(vec3f(1.0), mix(vec3f(scale_range.x), vec3f(scale_range.y), rand_y), scale);
    let scale1 = vec2f(scales.x);
    let scale2 = vec2f(scales.y);
    let scale3 = vec2f(scales.z);

    // Randomized offset.
    let offset1 = mix(vec2f(offset_range.x), vec2f(offset_range.y), rand1 * offset);
    let offset2 = mix(vec2f(offset_range.x), vec2f(offset_range.y), rand2 * offset);
    let offset3 = mix(vec2f(offset_range.x), vec2f(offset_range.y), rand3 * offset);

    var tile_data: HextileData;
    tile_data.weights = vec3f(w1, w2, w3);
    tile_data.rotations = rotations;

    // Get tile-local coords.
    tile_data.coords0 = (((coord - ctr1) * rm1) / scale1) + ctr1 + offset1;
    tile_data.coords1 = (((coord - ctr2) * rm2) / scale2) + ctr2 + offset2;
    tile_data.coords2 = (((coord - ctr3) * rm3) / scale3) + ctr3 + offset3;

    // Derivatives.
    let ddx_coord = dpdx(coord);
    let ddy_coord = dpdy(coord);
    tile_data.ddx0 = (ddx_coord * rm1) / scale1;
    tile_data.ddx1 = (ddx_coord * rm2) / scale2;
    tile_data.ddx2 = (ddx_coord * rm3) / scale3;
    tile_data.ddy0 = (ddy_coord * rm1) / scale1;
    tile_data.ddy1 = (ddy_coord * rm2) / scale2;
    tile_data.ddy2 = (ddy_coord * rm3) / scale3;

    return tile_data;
}
