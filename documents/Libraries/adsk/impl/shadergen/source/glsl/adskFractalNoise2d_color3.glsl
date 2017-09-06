#include "adsk/impl/shadergen/source/glsl/adskNoiseFunctions.glsl"

void adskFractalNoise2d_color3(vec2 position, float amplitude, float ratio, int octaves, float frequency, float frequencyRatio, vec2 distortion, float distortionRatio, out vec3 result)
{
	position *= 8;
	distortion *= distortionRatio;

	float simplex = 0.0f;
	for (int i=0; i < octaves+1; i++)
	{
		float noise = simplex_perlin2d(position+distortion) * pow(frequency, -amplitude*i);
		simplex += amplitude * noise;

		float tmp = position.x;
		position.x = position.y * frequency - 0.02f;
		position.y = tmp * frequency + 0.3f;

		amplitude *= ratio;
		frequency *= frequencyRatio;
		distortion *= distortionRatio;
	}

	simplex = 0.5f * simplex + 0.5f;

	result = vec3(simplex);
}
