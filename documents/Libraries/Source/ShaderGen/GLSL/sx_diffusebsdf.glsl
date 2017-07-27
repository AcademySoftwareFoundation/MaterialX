void sx_diffusebsdf(vec3 reflectance, float roughness, vec3 normal, out vec4 result)
{
	const vec3 Lo = -PS_IN.WorldView;
	vec3 L = vec3(0.0);
	const int numLights = min(ClampDynamicLights, 3);
	for (int ActiveLightIndex = 0; ActiveLightIndex < numLights; ++ActiveLightIndex)
	{
		vec3 LightPos = GetLightPos(ActiveLightIndex);
		vec3 LightDir = GetLightDir(ActiveLightIndex);
		vec3 LightVec = GetLightVectorFunction(ActiveLightIndex, LightPos, PS_IN.WorldPosition, LightDir);
		vec3 LightVecNorm = normalize(LightVec);
		vec3 Li = LightVecNorm;
		float cosThetaI = dot(normal, Li);
		if(cosThetaI < 0.0) {
			continue;
		}
		float cosThetaO = dot(normal, Lo);
		float t = dot(Li, Lo) - cosThetaI*cosThetaO;
		if (t>0.0) 
		{
			t /= max(cosThetaI, cosThetaO);
		}
		float cr = clamp(roughness, 0.0, M_PI*0.5);
		float sigma2 = cr*cr;
		float A = 1.0-0.5*sigma2/(sigma2+0.33) + 0.17*sigma2/(sigma2+0.13);
		float B = 0.45*(sigma2/(sigma2+0.09));
		vec3 LightContribution = LightContributionFunction(ActiveLightIndex, PS_IN.WorldPosition, LightVec);
		L += LightContribution * (A + B * t) * cosThetaI;
	}
	result = vec4(L * reflectance, 1.0);
}
