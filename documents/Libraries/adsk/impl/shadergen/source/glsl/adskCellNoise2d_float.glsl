#include "adsk/impl/shadergen/source/glsl/noise_functions.glsl"

void adskCellNoise2d_float(vec2 position, float amplitude, float frequency, vec2 distortion, float distortionRatio, out float result)
{
	position *= 8;
	position.x += distortion.x * distortionRatio;
	position.y += (distortion.y + 0.2) * distortionRatio;

	float noise = simplex_cellular2d(position * frequency);

	result = amplitude * noise;
}
