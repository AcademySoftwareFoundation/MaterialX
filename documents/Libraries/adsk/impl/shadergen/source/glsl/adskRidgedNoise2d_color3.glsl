#include "adsk/impl/shadergen/source/glsl/noise_functions.glsl"

void adskRidgedNoise2d_color3(vec2 position, float amplitude, float ratio, int octaves, float frequency, float frequencyRatio, vec2 distortion, float distortionRatio, out vec3 result)
{
	position *= 8;

	float offset = 1.0;
	float weight = 1.0;
	float signal = offset - abs(simplex_perlin2d(position));
	signal *= signal;
	distortion *= distortionRatio;

	float simplex = signal;
	for(int i=1; i<octaves; ++i)
	{
		position *= frequency;
		weight = clamp(signal*frequencyRatio, 0.0, 1.0);
		signal = offset - abs(simplex_perlin2d(position+distortion));
		signal *= signal*weight;
		simplex += signal * pow(frequency, -ratio) * amplitude;
		frequency *= frequencyRatio;
		distortion *= distortionRatio;
	}

	simplex = 0.5f * simplex + 0.5f;

	result = vec3(simplex);
}
