// --------------------------------- Lighting Uniforms --------------------------------------

uniform int ClampDynamicLights
<
    float UIMin = 0;
    float UISoftMin = 0;
    float UIMax = 99;
    float UISoftMax = 10;
    float UIStep = 1;
    string UIName = "Max Lights";
    string UIWidget = "Slider";
    string UIGroup = "Lighting";
> = 3;

uniform int Light0Type : LIGHTTYPE
<
    string UIName = "Light 0 Type";
    float UIMin = 0;
    float UIMax = 5;
    float UIStep = 1;
    string UIWidget = "None";
    string Object = "Light 0";
> = 3;

uniform int Light1Type : LIGHTTYPE
<
    string UIName = "Light 1 Type";
    float UIMin = 0;
    float UIMax = 5;
    float UIStep = 1;
    string UIWidget = "None";
    string Object = "Light 1";
> = 3;

uniform int Light2Type : LIGHTTYPE
<
    string UIName = "Light 2 Type";
    float UIMin = 0;
    float UIMax = 5;
    float UIStep = 1;
    string UIWidget = "None";
    string Object = "Light 2";
> = 3;

uniform vec3 Light0Color
<
    string UIName = "Light 0 Color";
    string UIWidget = "ColorPicker";
    string Object = "Light 0";
> = {1.0, 1.0, 1.0};

uniform vec3 Light1Color
<
    string UIName = "Light 1 Color";
    string UIWidget = "ColorPicker";
    string Object = "Light 1";
> = {1.0, 1.0, 1.0};

uniform vec3 Light2Color
<
    string UIName = "Light 2 Color";
    string UIWidget = "ColorPicker";
    string Object = "Light 2";
> = {1.0, 1.0, 1.0};

uniform float Light0Intensity : LIGHTINTENSITY
<
    string UIName = "Light 0 Intensity";
    float UIMin = 0;
    float UIStep = 0.1;
    string Object = "Light 0";
> = 1.0;

uniform float Light1Intensity : LIGHTINTENSITY
<
    string UIName = "Light 1 Intensity";
    float UIMin = 0;
    float UIStep = 0.1;
    string Object = "Light 1";
> = 1.0;

uniform float Light2Intensity : LIGHTINTENSITY
<
    string UIName = "Light 2 Intensity";
    float UIMin = 0;
    float UIStep = 0.1;
    string Object = "Light 2";
> = 1.0;

uniform vec3 Light0Pos : POSITION
<
    string UIName = "Light 0 Position";
    string Space = "World";
    string Object = "Light 0";
> = {1.0, 1.0, 1.0};

uniform vec3 Light1Pos : POSITION
<
    string UIName = "Light 1 Position";
    string Space = "World";
    string Object = "Light 1";
> = { -1.0, -1.0, 1.0 };

uniform vec3 Light2Pos : POSITION
<
    string UIName = "Light 2 Position";
    string Space = "World";
    string Object = "Light 2";
> = { 1.0, -1.0, -1.0 };

uniform vec3 Light0Dir : DIRECTION
<
    string UIName = "Light 0 Direction";
    string Space = "World";
    string Object = "Light 0";
> = {-1.0, -1.0, 0.0};

uniform vec3 Light1Dir : DIRECTION
<
    string UIName = "Light 1 Direction";
    string Space = "World";
    string Object = "Light 1";
> = {-1.0, 1.0, 0.0};

uniform vec3 Light2Dir : DIRECTION
<
    string UIName = "Light 2 Direction";
    string Space = "World";
    string Object = "Light 2";
> = {0.0, -1.0, 1.0};

uniform float Light0Attenuation : DECAYRATE
<
    string UIName = "Light 0 Decay";
    float UIMin = 0;
    float UIStep = 1;
    string Object = "Light 0";
> = 0.01;

uniform float Light1Attenuation : DECAYRATE
<
    string UIName = "Light 1 Decay";
    float UIMin = 0;
    float UIStep = 1;
    string Object = "Light 1";
> = 0.01;

uniform float Light2Attenuation : DECAYRATE
<
    string UIName = "Light 2 Decay";
    float UIMin = 0;
    float UIStep = 1;
    string Object = "Light 2";
> = 0.01;

uniform float Light0ConeAngle : HOTSPOT
<
    string UIName = "Light 0 Cone Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Space = "World";
    string Object = "Light 0";
> = 0.46;

uniform float Light1ConeAngle : HOTSPOT
<
    string UIName = "Light 1 Cone Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Space = "World";
    string Object = "Light 1";
> = 0.46;

uniform float Light2ConeAngle : HOTSPOT
<
    string UIName = "Light 2 Cone Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Space = "World";
    string Object = "Light 2";
> = 0.46;

uniform float Light0Falloff : FALLOFF
<
    string UIName = "Light 0 Penumbra Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Object = "Light 0";
> = 0.7;

uniform float Light1Falloff : FALLOFF
<
    string UIName = "Light 1 Penumbra Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Object = "Light 1";
> = 0.7;

uniform float Light2Falloff : FALLOFF
<
    string UIName = "Light 2 Penumbra Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Object = "Light 2";
> = 0.7;

uniform bool Light0ShadowOn : SHADOWFLAG
<
    string UIName = "Light 0 Casts Shadow";
    string Object = "Light 0";
> = false;

uniform bool Light1ShadowOn : SHADOWFLAG
<
    string UIName = "Light 1 Casts Shadow";
    string Object = "Light 1";
> = false;

uniform bool Light2ShadowOn : SHADOWFLAG
<
    string UIName = "Light 2 Casts Shadow";
    string Object = "Light 2";
> = false;

uniform mat4 Light0ViewPrj : SHADOWMAPMATRIX
<
    string Object = "Light 0";
    string UIName = "Light 0 Matrix";
    string UIWidget = "None";
>;

uniform mat4 Light1ViewPrj : SHADOWMAPMATRIX
<
    string Object = "Light 1";
    string UIName = "Light 1 Matrix";
    string UIWidget = "None";
>;

uniform mat4 Light2ViewPrj : SHADOWMAPMATRIX
<
    string Object = "Light 2";
    string UIName = "Light 2 Matrix";
    string UIWidget = "None";
>;

uniform vec3 Light0ShadowColor : SHADOWCOLOR
<
    string UIName = "Light 0 Shadow Color";
    string Object = "Light 0";
> = {0, 0, 0};

uniform vec3 Light1ShadowColor : SHADOWCOLOR
<
    string UIName = "Light 1 Shadow Color";
    string Object = "Light 1";
> = {0, 0, 0};

uniform vec3 Light2ShadowColor : SHADOWCOLOR
<
    string UIName = "Light 2 Shadow Color";
    string Object = "Light 2";
> = {0, 0, 0};

uniform texture2D Specular_IBL_file_texture : SourceTexture;
uniform sampler2D Specular_IBL_file = sampler_state
{
    Texture = <Specular_IBL_file_texture>;
};

uniform texture2D Irradiance_IBL_file_texture : SourceTexture;
uniform sampler2D Irradiance_IBL_file = sampler_state
{
    Texture = <Irradiance_IBL_file_texture>;
};

// --------------------------------- Lighting Functions -------------------------------------

GLSLShader LightingFunctions
{
    int GetLightType(int ActiveLightIndex)
    {
        if (ActiveLightIndex == 0)
            return Light0Type;
        else if (ActiveLightIndex == 1)
            return Light1Type;
        else
            return Light2Type;
    }

    vec3 GetLightColor(int ActiveLightIndex)
    {
        if (ActiveLightIndex == 0)
            return Light0Color;
        else if (ActiveLightIndex == 1)
            return Light1Color;
        else
            return Light2Color;
    }

    float GetLightIntensity(int ActiveLightIndex)
    {
        if (ActiveLightIndex == 0)
            return Light0Intensity;
        else if (ActiveLightIndex == 1)
            return Light1Intensity;
        else
            return Light2Intensity;
    }

    vec3 GetLightPos(int ActiveLightIndex)
    {
        if (ActiveLightIndex == 0)
            return Light0Pos;
        else if (ActiveLightIndex == 1)
            return Light1Pos;
        else
            return Light2Pos;
    }

    vec3 GetLightDir(int ActiveLightIndex)
    {
        if (ActiveLightIndex == 0)
            return normalize(Light0Dir);
        else if (ActiveLightIndex == 1)
            return normalize(Light1Dir);
        else
            return normalize(Light2Dir);
    }

    float GetLightAttenuation(int ActiveLightIndex)
    {
        if (ActiveLightIndex == 0)
            return Light0Attenuation;
        else if (ActiveLightIndex == 1)
            return Light1Attenuation;
        else
            return Light2Attenuation;
    }

    float GetLightConeAngle(int ActiveLightIndex)
    {
        if (ActiveLightIndex == 0)
            return Light0ConeAngle;
        else if (ActiveLightIndex == 1)
            return Light1ConeAngle;
        else
            return Light2ConeAngle;
    }

    float GetLightFalloff(int ActiveLightIndex)
    {
        if (ActiveLightIndex == 0)
            return Light0Falloff;
        else if (ActiveLightIndex == 1)
            return Light1Falloff;
        else
            return Light2Falloff;
    }

    bool GetLightShadowOn(int ActiveLightIndex)
    {
        if (ActiveLightIndex == 0)
            return Light0ShadowOn;
        else if (ActiveLightIndex == 1)
            return Light1ShadowOn;
        else
            return Light2ShadowOn;
    }

    mat4 GetLightViewPrj(int ActiveLightIndex)
    {
        if (ActiveLightIndex == 0)
            return Light0ViewPrj;
        else if (ActiveLightIndex == 1)
            return Light1ViewPrj;
        else
            return Light2ViewPrj;
    }

    vec3 GetLightShadowColor(int ActiveLightIndex)
    {
        if (ActiveLightIndex == 0)
            return Light0ShadowColor;
        else if (ActiveLightIndex == 1)
            return Light1ShadowColor;
        else
            return Light2ShadowColor;
    }

    vec3 GetLightVectorFunction(int ActiveLightIndex, vec3 LightPosition, vec3 VertexWorldPosition, vec3 LightDirection)
    {
        int _LightType = GetLightType(ActiveLightIndex);
        bool IsDirectionalLight = (_LightType == 4);
        vec3 LerpOp = mix((LightPosition - VertexWorldPosition), -(LightDirection), float(IsDirectionalLight));
        return LerpOp;
    }

    float LightDecayFunction(int ActiveLightIndex, vec3 LightVectorUN, float Attenuation)
    {
        float DecayContribution116 = 1.0;
        if (Attenuation > 0.001)
        {
            float PowOp = pow(length(LightVectorUN), Attenuation);
            float DivOp = (1.0 / PowOp);
            DecayContribution116 = DivOp;
        }
        return DecayContribution116;
    }

    float LightConeAngleFunction(int ActiveLightIndex, vec3 LightVector, vec3 LightDirection, float ConeAngle, float ConeFalloff)
    {
        float CosOp = cos(max(ConeFalloff, ConeAngle));
        float DotOp = dot(LightVector, -(LightDirection));
        float SmoothStepOp = smoothstep(CosOp, cos(ConeAngle), DotOp);
        return SmoothStepOp;
    }

    vec3 LightContributionFunction(int ActiveLightIndex, vec3 VertexWorldPosition, vec3 LightVectorUN)
    {
        float _LightIntensity = GetLightIntensity(ActiveLightIndex);
        int _LightType = GetLightType(ActiveLightIndex);
        bool IsDirectionalLight = (_LightType == 4);
        float DecayMul162 = 1.0;
        if (!IsDirectionalLight)
        {
            float _LightAttenuation = GetLightAttenuation(ActiveLightIndex);
            DecayMul162 = LightDecayFunction(ActiveLightIndex, LightVectorUN, _LightAttenuation);
        }
        bool IsSpotLight = (_LightType == 2);
        float ConeMul164 = 1.0;
        if (IsSpotLight)
        {
            vec3 NormOp = normalize(LightVectorUN);
            vec3 _LightDir = GetLightDir(ActiveLightIndex);
            float _LightConeAngle = GetLightConeAngle(ActiveLightIndex);
            float _LightFalloff = GetLightFalloff(ActiveLightIndex);
            ConeMul164 = LightConeAngleFunction(ActiveLightIndex, NormOp, _LightDir, _LightConeAngle, _LightFalloff);
        }
        float ShadowMul165 = 1.0;
//      bool _LightShadowOn = GetLightShadowOn(ActiveLightIndex);
//      if (_LightShadowOn)
//      {
//          mat4 _LightViewPrj = GetLightViewPrj(ActiveLightIndex);
//          ShadowMapOutput ShadowMap178 = ShadowMapFunction(ActiveLightIndex, _LightViewPrj, 0.01, VertexWorldPosition);
//          vec3 _LightShadowColor = GetLightShadowColor(ActiveLightIndex);
//          float ShadowColorMix = mix(ShadowMap178.LightGain, 1.0, float(_LightShadowColor.x));
//          ShadowMul165 = ShadowColorMix;
//      }
        float DecayShadowConeMul = ((ShadowMul165 * ConeMul164) * DecayMul162);
        vec3 _LightColor = GetLightColor(ActiveLightIndex);
        vec3 result = ((_LightColor * DecayShadowConeMul) * _LightIntensity);
        return result;
    }

    vec2 GetSphericalCoords(vec3 vec)
    {
        float v = acos(clamp(vec.z, -1.0, 1.0));
        float u = clamp(vec.x / sin(v), -1.0, 1.0);
        if (vec.y >= 0.0)
            u = acos(u);
        else
            u = 2 * M_PI - acos(u);
        return vec2(u / (2 * M_PI), v / M_PI);
    }

    vec3 LatLongMapLookup(vec3 dir, float lodBias, sampler2D sampler)
    {
        vec2 res = textureSize(sampler, 0);
        if (res.x > 0)
        {
            vec2 uv = GetSphericalCoords(dir);
            int levels = 1 + int(floor(log2(max(res.x, res.y))));
            float lod = lodBias * levels;
            return textureLod(sampler, uv, lod).rgb;
        }
        return vec3(0.0);
    }

    vec3 SpecularEnvironment(vec3 normal, vec3 view, float roughness)
    {
        vec3 dir = reflect(-view, normal);
        // Y is up vector
        dir = vec3(dir.x, -dir.z, dir.y);

        return LatLongMapLookup(dir, roughness, Specular_IBL_file);
    }

    vec3 IrradianceEnvironment(vec3 normal)
    {
        return LatLongMapLookup(normal, 1.0, Irradiance_IBL_file);
    }
}
