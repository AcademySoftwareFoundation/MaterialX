void sx_diffusebsdf(vec3 L, vec3 V, vec3 reflectance, float roughness, vec3 normal, out BSDF result)
{
	float cosTheta = max(dot(normal, L), 0.0);
	result.fr = reflectance * M_PI_INV * cosTheta;
	result.ft = vec3(0.0);
}
