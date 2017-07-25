// Arnold standard shader diffuse
vec3 standardShaderDiffuse(vec3 N, vec3 L, vec3 inDiffuse)
{
    float NDL = saturate(dot(N, L));
    if (NDL > 0.0f)
        return inDiffuse * NDL;
    else
        return vec3(0.0f, 0.0f, 0.0f);
}

float SQR(float a)
{
    return a * a;
}

// Arnold standard shader specular
vec3 standardShaderSpecular(
    vec3 N,
    vec3 SL, // Same as diffuse for now
    vec3 V, // PS_IN.Vw
    vec3 specular,
    float roughness)
{

    float LdotN = saturate(dot(SL, N));
    if (LdotN == 0.0f)
        return vec3(0.0f, 0.0f, 0.0f);
    float VdotN = saturate(dot(V, N));
    if (VdotN == 0.0f)
        return vec3(0.0f, 0.0f, 0.0f);

    vec3 iPlusO = SL + V;
    vec3 microfacet = normalize(iPlusO);

    float LdotM = saturate(dot(SL, microfacet));
    if (LdotM == 0.0f) return vec3(0.0f, 0.0f, 0.0f);
    float VdotM = saturate(dot(V, microfacet));
    if (VdotM == 0.0f) return vec3(0.0f, 0.0f, 0.0f);
    float MdotN = saturate(dot(microfacet, N));
    if (MdotN == 0.0f) return vec3(0.0f, 0.0f, 0.0f);

    float LdotN2 = SQR(LdotN);
    float VdotN2 = SQR(VdotN);
    float MdotN2 = SQR(MdotN);
    float MdotN4 = SQR(MdotN2);

    float rx = SQR(saturate(roughness));

    float rx2 = SQR(rx);
    float tanThetaL = sqrt(1.0f - LdotN2) / LdotN;
    float tanThetaV = sqrt(1.0f - VdotN2) / VdotN;
    float tan2ThetaM = (1.0f - MdotN2) / MdotN2;

    float a_im = 1.0f / (rx * tanThetaL);
    float a_im2 = SQR(a_im);
    float g1im = (a_im < 1.6f) ? (3.535f * a_im + 2.181f * a_im2) / (1.0f + 2.276f * a_im + 2.577f * a_im2) : 1.0f;
    float a_om = 1.0f / (rx * tanThetaV);
    float a_om2 = SQR(a_om);
    float g1om = (a_om < 1.6f) ? (3.535f * a_om + 2.181f * a_om2) / (1.0f + 2.276f * a_om + 2.577f * a_om2) : 1.0f;
    float G = g1im * g1om;

    float D = (rx2 > 0.0f) ? exp(-tan2ThetaM / rx2) / (3.14159f * rx2 * MdotN4) : 0.0f;

    return max(vec3(0.0f, 0.0f, 0.0f), ((G * D) / (4.f * LdotN * VdotN)) * specular);
}

// Arnold standard emission
vec3 standardShaderEmission(
    vec3 emissionColor,
    float emission)
{
    return emissionColor * emission;
}

// Diffuse computation computation
vec3 computeDiffuse(float roughness, vec3 normal)
{
    vec3 L = vec3(0.0);
    const int numLights = min(ClampDynamicLights, 3);
    for (int ActiveLightIndex = 0; ActiveLightIndex < numLights; ++ActiveLightIndex)
    {
        vec3 LightPos = GetLightPos(ActiveLightIndex);
        vec3 LightDir = GetLightDir(ActiveLightIndex);
        vec3 LightVec = GetLightVectorFunction(ActiveLightIndex, LightPos, PS_IN.WorldPosition, LightDir);
        vec3 LightVecNorm = normalize(LightVec);

        vec3 LightContribution = LightContributionFunction(ActiveLightIndex, PS_IN.WorldPosition, LightVec);
        L += standardShaderDiffuse(normal, LightVecNorm, LightContribution);
    }
    return L;
}

// Specular contribution computation
void computeSpecular(float roughness, vec3 normal, vec3 view, out vec3 result)
{
    vec3 L = vec3(0.0);
    const int numLights = min(ClampDynamicLights, 3);
    for (int ActiveLightIndex = 0; ActiveLightIndex < numLights; ++ActiveLightIndex)
    {
        vec3 LightPos = GetLightPos(ActiveLightIndex);
        vec3 LightDir = GetLightDir(ActiveLightIndex);
        vec3 LightVec = GetLightVectorFunction(ActiveLightIndex, LightPos, PS_IN.WorldPosition, LightDir);
        vec3 LightVecNorm = normalize(LightVec);

        vec3 LightContribution = LightContributionFunction(ActiveLightIndex, PS_IN.WorldPosition, LightVec);

        vec3 specular = standardShaderSpecular(normal, LightVecNorm, view, LightContribution, roughness);
        L += specular;
    }
    result = L;
}

// Arnold standard shader combiner
vec3 standardShaderCombiner(
    vec3 diffuseInput,
    vec3 specularInput,
    vec3 color,
    float diffuseRoughness,
    vec3 specularColor,
    float specularRoughness,
    float metalness,
    float transmission,
    vec3 transmissionColor,
    float transmissionDepth,
    float transmissionRoughness,
    bool thinWalled,
    vec3 opacity,
    float subsurface,
    vec3 subsurfaceColor,
    float coat,
    vec3 coatColor,
    float coatIOR,
    float directDiffuse,
    float directSpecular,
    bool specularFresnel,
    bool FresnelAffectDiffuse,
    bool FresnelUseIOR,
    float Ksn,
    float Ks,
    float Kd,
    float IOR,
    vec3 IrradianceEnv,
    vec3 SpecularEnv,
    vec3 N,
    vec3 V)
{
    vec3 result;

    float fresnel = 1.0;
    float coatFresnel = 1.0;
    float transFresnel = 1.0;
    float coatFresnelWeight = 0.0;
    float clampedIOR = max(1.0, IOR);

    vec3 specularAmount = (specularInput + SpecularEnv);
    float NV = dot(N, -V);

    // compute the fresnel factors for the specular, coat, and transmission layers
    if (NV < 0.0f)
    {
        coatFresnelWeight = (coatIOR - 1) / (coatIOR + 1);
        coatFresnelWeight *= coatFresnelWeight;
        float fresnelWeight = (clampedIOR - 1) / (clampedIOR + 1);
        fresnelWeight *= fresnelWeight;

        float temp = 1.0f + NV;
        transFresnel = fresnelWeight + temp * (1.0f - fresnelWeight);

        float fweight = temp * temp;
        fweight *= fweight;
        fweight *= temp;

        fresnel = fresnelWeight + fweight * (1.0f - fresnelWeight);
        coatFresnel = coatFresnelWeight + fweight * (1.0f - coatFresnelWeight);
        coatFresnelWeight = fweight;
    }

    // compute the coat color
    vec3 cc = lerp(vec3(1.0, 1.0, 1.0), coatColor, coat);

    // Compute the coat specular color.
    // In Arnold the secondary coat adds white specular highlights.
    vec3 coatSpecular = specularAmount * coatFresnel * coat;

    // compute the diffuse
    vec3 diffuse = color * Kd;

    // include the sub-surface color
    // TODO: Provide sub-surface scattering
    diffuse = lerp(diffuse, subsurfaceColor, subsurface * (1.0 - metalness));

    // apply the diffuse roughness
    diffuse = lerp(diffuse, diffuse * 0.8, diffuseRoughness);

    // compute the specular.  Specular goes to black as the metalness increases.
    // metalness has its own specularity
    vec3 specular = specularColor * specularAmount * Ks;
    specular *= 1.0 - metalness;

    // compute the metal color
    vec3 metalColor = metalness * specularAmount * diffuse * cc * (1.0 - coatFresnelWeight);
    vec3 metalSpecular = metalness * specularAmount * coatFresnelWeight;

    // energy conservation for the metal color
    metalColor *= 1.0 - (coatFresnel*coat);

    //adjust the specular using the fresnel factor.
    specular *= fresnel;

    // compute the transmission  
    transmissionRoughness = max(transmissionRoughness, specularRoughness);
    float transp = pow(1.0 - transFresnel, 3.0 + 7.0*(1.0 - transmissionRoughness));

    // fake some translucency   
    if (!thinWalled)
        transp = lerp(transp, 1.0 - abs(transp - 0.5)*2.0, max(clampedIOR, 1.4) - 1.0);

    // Compute the transparency amount.  Roughness makes things less transparent.
    vec3 transAmount = (transmissionColor * transp) * (1.0 - 0.85*transmissionRoughness);

    // modify the transparent amount so clear is clear no matter what angle you look at it from.
    float shortestAxis = min(min(transmissionColor.x, transmissionColor.y), transmissionColor.z);
    transAmount = lerp(transAmount, transmissionColor, shortestAxis);

    vec3 tt = transmissionColor * transp;
    tt *= (1.0 - transAmount) * clampedIOR;

    // apply the affects of transmission
    diffuse = lerp(diffuse, tt, transmission);

    // include the secondary coat     
    diffuse *= cc;

    // add the diffuse lighting
    diffuse *= (diffuseInput + IrradianceEnv);

    // account for opacity (remove color where transparent)
    diffuse *= opacity;

    // energy conservation for metalness and coat contributions
    diffuse *= 1.0 - min(1.0, max(coatFresnel * coat, metalness));

    // lower the specular in the straight-on angles when the roughness is low.   
    specular *= lerp(1.0 - transp, 1.0, transmissionRoughness * (1.0 - transmission));

    result = diffuse + specular + metalColor + metalSpecular + coatSpecular;

    // None of these results are required outputs for now
    //result.outTransparency = max((1.0 - metalness) * transmission * transAmount, (1.0 - opacity));
    //result.outGlowColor = vec3(0.0f, 0.0f, 0.0f);
    //result.outMatteOpacity = vec3(-1.0e+06f, -1.0e+06f, -1.0e+06f);
    //result.outSurfaceFinal = vec4(result.outColor, 1.0f);
    return result;
}

void adskSurface(
    float base,
    vec3 base_color,
    float diffuse_roughness,
    float specular,
    vec3 specular_color,
    float specular_roughness,
    float specular_IOR,
    float specular_anisotropy,
    float specular_rotation,
    float metalness,
    float transmission,
    vec3 transmission_color,
    float transmission_depth,
    vec3 transmission_scatter,
    float transmission_scatter_anisotropy,
    float transmission_dispersion,
    float transmission_extra_roughness,
    float subsurface,
    vec3 subsurface_color,
    vec3 subsurface_radius,
    float subsurface_scale,
    bool thin_walled,
    vec3 normal,
    vec3 tangent,
    float coat,
    vec3 coat_color,
    float coat_roughness,
    float coat_IOR,
    vec3 coat_normal,
    float thin_film_thickness,
    float thin_film_IOR,
    float emission,
    vec3 emission_color,
    vec3 opacity,
    bool caustics,
    bool internal_reflections,
    bool exit_to_background,
    float indirect_diffuse,
    float indirect_specular,
    out surfaceshader result)
{
    vec3 worldNormal = normalize(normal);

    // Compute diffuse, and texture map with a checker
    vec3 _diffuse = vec3(0.0);
    _diffuse = computeDiffuse(diffuse_roughness, worldNormal);

    // Compute specular
    vec3 worldView = normalize(PS_IN.WorldView); // Not correct. Which one to use
    vec3 _specular = vec3(0.0);
    computeSpecular(specular_roughness, worldNormal, worldView, _specular);

    // Compute emission
    result.edf = standardShaderEmission(emission_color, emission);

    // Extra inputs
    float directDiffuse = 1.0;
    float directSpecular = 1.0;
    bool specularFresnel = false;
    bool FresnelAffectDiffuse = false;
    bool FresnelUseIOR = false;
    float Ksn = 0.0;
    vec3 IrradianceEnv = vec3(0.000000, 0.000000, 0.000000);
    vec3 SpecularEnv = EnvironmentLight(worldNormal, worldView, specular_roughness);

    // Compute total bsdf
    result.bsdf = standardShaderCombiner(
        _diffuse,
        _specular,
        base_color,
        diffuse_roughness,
        specular_color,
        specular_roughness,
        metalness,
        transmission,
        transmission_color,
        transmission_depth,
        transmission_extra_roughness,
        thin_walled,
        opacity,
        subsurface,
        subsurface_color,
        coat,
        coat_color,
        coat_IOR,
        // Extra params
        directDiffuse,
        directSpecular,
        specularFresnel,
        FresnelAffectDiffuse,
        FresnelUseIOR,
        Ksn,
        specular, // Ks,
        base, // Kd,
        specular_IOR, //IOR,
        IrradianceEnv,
        SpecularEnv,
        worldNormal,
        worldView);

    result.ior = specular_IOR;
}
