#include "mx_noise.glsl"

// "Fast Random Rotation Matrices" by James Arvo, Graphics Gems3 P.117
vec3 mx_rotate_flake(vec3 p, vec3 i)
{
    float theta = M_PI * 2.0 * i.x;
    float phi = M_PI * 2.0 * i.y;
    float z = i.z * 2.0;

    float r = sqrt(z);
    float vx = sin(phi) * r;
    float vy = cos(phi) * r;
    float vz = sqrt(2.0 - z);

    float s_theta = sin(theta);
    float c_theta = cos(theta);
    float sx = vx * c_theta - vy * s_theta;
    float sy = vx * s_theta + vy * c_theta;

    mat3 m = mat3(
        vx * sx - c_theta, vx * sy - s_theta, vx * vz,
        vy * sx + s_theta, vy * sy - c_theta, vy * vz,
        vz * sx          , vz * sy          , 1.0 - z
    );

    return m * p;
}

// compute a flake probability for a given flake coverage density x
float mx_flake_density_to_probability(float x)
{
    // constants for numerical fitted curve to observed flake noise density behavior
    const vec4 abcd = vec4(-26.19771808f, 26.39663835f, 85.53857017f, -102.35069432f);
    const vec2 ef = vec2(-101.42634862f, 118.45082288f);
    float xx = x * x;

    return (abcd.x * xx + abcd.y * x) / (abcd.z * xx * x + abcd.w * xx  + ef.x * x + ef.y);
}

void mx_flake(
    float size,
    float roughness,
    float coverage,
    vec3 position,
    vec3 normal,
    vec3 tangent,
    vec3 bitangent,
    out int id,
    out float rand,
    out float presence,
    out vec3 flakenormal
)
{
    float probability = mx_flake_density_to_probability(clamp(coverage, 0.0, 1.0));
    const float flake_diameter = 1.5 / sqrt(3.0);

    vec3 P = position / vec3(size);
    vec3 base_P = floor(P);

    // flake priority in [0..1], 0: no flake, flakes with higher priority shadow flakes "below" them
    float flake_priority = 0.0;
    vec3 flake_cell = vec3(0.0);

    // Examine the 3x3x3 neighborhood of cells around the sample position, selecting the
    // highest-priority overlapping flake.
    for (int i = -1; i < 2; ++i)
    {
        for (int j = -1; j < 2; ++j)
        {
            for (int k = -1; k < 2; ++k)
            {
                vec3 cell_pos = base_P + vec3(i, j, k);

                vec3 PP = P - cell_pos - vec3(0.5);
                if (dot(PP, PP) >= flake_diameter * flake_diameter * 3.0)
                    continue;

                if (mx_cell_noise_float(cell_pos) > probability)
                    continue;

                float priority = mx_cell_noise_float(vec4(cell_pos, 3.0));
                if (priority < flake_priority)
                    continue;

                vec3 rot = mx_cell_noise_vec3(cell_pos);
                PP = mx_rotate_flake(PP, rot);

                if (abs(PP.x) <= flake_diameter &&
                    abs(PP.y) <= flake_diameter &&
                    abs(PP.z) <= flake_diameter)
                {
                    flake_priority = priority;
                    flake_cell = cell_pos;
                }
            }
        }
    }

    if (flake_priority <= 0.0)
    {
        // no flake
        id = 0;
        rand = 0.0;
        presence = 0.0;
        flakenormal = normal;
        return;
    }

    // create a flake normal by importance sampling a microfacet distribution with given roughness
    vec3 flake_noise = mx_cell_noise_vec3(vec4(flake_cell, 2.0));
    float xi0 = flake_noise.x;
    float xi1 = flake_noise.y;

    rand = flake_noise.z;
    id = int(rand * 2147483647.0);
    presence = flake_priority;

    float phi = M_PI * 2.0 * xi0;
    float tan_theta = roughness * roughness * sqrt(xi1) / sqrt(1.0 - xi1);  // GGX
    float sin_theta = tan_theta / sqrt(1.0 + tan_theta * tan_theta);
    float cos_theta = sqrt(1.0 - sin_theta * sin_theta);

    flakenormal = tangent * cos(phi) * sin_theta +
                  bitangent * sin(phi) * sin_theta +
                  normal * cos_theta;
    flakenormal = normalize(flakenormal);
}
