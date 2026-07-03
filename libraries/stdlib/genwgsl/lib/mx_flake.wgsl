#include "mx_noise.wgsl"

// "Fast Random Rotation Matrices" by James Arvo, Graphics Gems3 P.117
fn mx_rotate_flake(p: vec3f, i: vec3f) -> vec3f {
    let theta = M_PI * 2.0 * i.x;
    let phi = M_PI * 2.0 * i.y;
    let z = i.z * 2.0;

    let r = sqrt(z);
    let vx = sin(phi) * r;
    let vy = cos(phi) * r;
    let vz = sqrt(2.0 - z);

    let s_theta = sin(theta);
    let c_theta = cos(theta);
    let sx = vx * c_theta - vy * s_theta;
    let sy = vx * s_theta + vy * c_theta;

    let m = mat3x3f(
        vx * sx - c_theta, vx * sy - s_theta, vx * vz,
        vy * sx + s_theta, vy * sy - c_theta, vy * vz,
        vz * sx          , vz * sy          , 1.0 - z
    );

    return m * p;
}

fn mx_flake_density_to_probability(x: f32) -> f32 {
    let abcd = vec4f(-26.19771808, 26.39663835, 85.53857017, -102.35069432);
    let ef = vec2f(-101.42634862, 118.45082288);
    let xx = x * x;

    return (abcd.x * xx + abcd.y * x) / (abcd.z * xx * x + abcd.w * xx + ef.x * x + ef.y);
}

fn mx_flake(
    size: f32,
    roughness: f32,
    coverage: f32,
    position: vec3f,
    normal: vec3f,
    tangent: vec3f,
    bitangent: vec3f,
    id: ptr<function, i32>,
    rand_out: ptr<function, f32>,
    presence: ptr<function, f32>,
    flakenormal: ptr<function, vec3f>
) {
    let probability = mx_flake_density_to_probability(clamp(coverage, 0.0, 1.0));
    let flake_diameter = 1.5 / sqrt(3.0);

    let P = position / vec3f(size);
    let base_P = floor(P);

    var flake_priority: f32 = 0.0;
    var flake_cell: vec3f = vec3f(0.0);

    for (var i: i32 = -1; i < 2; i = i + 1) {
        for (var j: i32 = -1; j < 2; j = j + 1) {
            for (var k: i32 = -1; k < 2; k = k + 1) {
                let cell_pos = base_P + vec3f(f32(i), f32(j), f32(k));

                let PP_pre = P - cell_pos - vec3f(0.5);
                if (dot(PP_pre, PP_pre) >= flake_diameter * flake_diameter * 3.0) {
                    continue;
                }

                if (mx_cell_noise_float_vec3(cell_pos) > probability) {
                    continue;
                }

                let priority = mx_cell_noise_float_vec4(vec4f(cell_pos, 3.0));
                if (priority < flake_priority) {
                    continue;
                }

                let rot = mx_cell_noise_vec3_vec3(cell_pos);
                let PP = mx_rotate_flake(PP_pre, rot);

                if (abs(PP.x) <= flake_diameter &&
                    abs(PP.y) <= flake_diameter &&
                    abs(PP.z) <= flake_diameter) {
                    flake_priority = priority;
                    flake_cell = cell_pos;
                }
            }
        }
    }

    if (flake_priority <= 0.0) {
        *id = 0;
        *rand_out = 0.0;
        *presence = 0.0;
        *flakenormal = normal;
        return;
    }

    let flake_noise = mx_cell_noise_vec3_vec4(vec4f(flake_cell, 2.0));
    let xi0 = flake_noise.x;
    let xi1 = flake_noise.y;

    *rand_out = flake_noise.z;
    *id = i32(*rand_out * 2147483647.0);
    *presence = flake_priority;

    let phi = M_PI * 2.0 * xi0;
    let tan_theta = roughness * roughness * sqrt(xi1) / sqrt(1.0 - xi1);
    let sin_theta = tan_theta / sqrt(1.0 + tan_theta * tan_theta);
    let cos_theta = sqrt(1.0 - sin_theta * sin_theta);

    *flakenormal = normalize(
        tangent * cos(phi) * sin_theta +
        bitangent * sin(phi) * sin_theta +
        normal * cos_theta
    );
}
