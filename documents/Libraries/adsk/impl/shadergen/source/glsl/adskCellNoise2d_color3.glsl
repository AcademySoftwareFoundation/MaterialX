#include "adsk/impl/shadergen/source/glsl/adsk_noise_functions.glsl"

void adskCellNoise2d_color3(vec2 position, float amplitude, float frequency, vec2 distortion, float distortionRatio, out vec3 result)
{
	position *= 8;
	position.x += distortion.x * distortionRatio;
	position.y += (distortion.y + 0.2) * distortionRatio;

	float noise = simplex_cellular2d(position * frequency);

	result = vec3(amplitude * noise);
}
