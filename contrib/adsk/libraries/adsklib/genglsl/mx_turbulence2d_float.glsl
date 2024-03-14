#include "../../stdlib/genglsl/lib/mx_noise.glsl"

void mx_turbulence2d_float(float amplitude, float octaves, vec2 texcoord, out float result)
{
    float sum = 0.0;
    float scale = 1.0;
    
    // Accumulate the required number of octaves of noise.
    float i = 0.0;
    for (i = octaves; i >= 1.0; i -= 1.0) {
        float value = mx_perlin_noise_float(texcoord * scale) / scale;
        sum += abs(value);
        scale *= 2.0;
    }

    // Add a portion of the remaining octave, if any.
    if (i > 0.0) {
        float value = mx_perlin_noise_float(texcoord * scale) / scale;
        sum += abs(value) * i;
    }

	result = sum * amplitude;
}
