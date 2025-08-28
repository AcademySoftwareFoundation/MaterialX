#include "../../stdlib/genglsl/lib/mx_noise.glsl"
// 1 dimensional gradient function for 1D perlin noise
float mx_gradient_float(uint hash, float x)
{
    uint  h = hash & 15u;
    float g = 1 + (h & 7u);
    return mx_negate_if(g, bool(h&8u)) * x; 
}

// Scaling factors to normalize the result of gradients above.
// These factors were experimentally calculated to be:
//    1D:   0.2500
//    2D:   0.6616
//    3D:   0.9820
float mx_gradient_scale1d(float v) { return 0.2500 * v; }

// 1D perlin noise
float mx_perlin_noise_float(float p)
{
    int X;
    float fx = mx_floorfrac(p, X);
    float u = mx_fade(fx);
    // mix in glsl is equivalent to lerp
    float result = mix(
        mx_gradient_float(mx_hash_int(X  ), fx ),
        mx_gradient_float(mx_hash_int(X+1), fx-1.0),
        u);
    return mx_gradient_scale1d(result);
}
void mx_noise1d_float(float amplitude, float pivot, float p, out float result)
{
    float value = mx_perlin_noise_float(p);
    result = value * amplitude + pivot;
}
