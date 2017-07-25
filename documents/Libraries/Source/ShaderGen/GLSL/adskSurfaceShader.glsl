void adskSurfaceShader(vec3 bsdf, vec3 edf, float ior, out surfaceshader result)
{
	result.bsdf = bsdf;
	result.edf  = edf;
	result.ior  = ior;
}
