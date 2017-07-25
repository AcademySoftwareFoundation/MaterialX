#include <MaterialXShaderGen/ShaderGenerators/OgsFxShaderGenerator.h>
#include <sstream>

namespace
{

//
// TODO: Make the inclusion of these shader blocks data driven
//

const char* kShaderBlock_Matrices =
    "// ----------------------------------- Transformation Matrices --------------------------------------\n"
    "\n"
    "// Transform object vertices to world-space\n"
    "uniform mat4 gWorldXf : World < string UIWidget = \"None\"; >;\n"
    "\n"
    "// Transform object normals, tangents, & binormals to world-space\n"
    "uniform mat4 gWorldITXf : WorldInverseTranspose < string UIWidget = \"None\"; >;\n"
    "\n"
    "// Transform object vertices to view space and project them in perspective\n"
    "uniform mat4 gWvpXf : WorldViewProjection < string UIWidget = \"None\"; >;\n"
    "\n"
    "// Provide tranform from 'view' or 'eye' coords back to world-space\n"
    "uniform mat4 gViewIXf : ViewInverse < string UIWidget = \"None\"; >;\n"
    "\n";

const char* kShaderBlock_Lights =
    "// ----------------------------------- Light Parameters --------------------------------------\n"
    "\n"
    "uniform int ClampDynamicLights\n"
    "<\n"
    "	float UIMin = 0;\n"
    "	float UISoftMin = 0;\n"
    "	float UIMax = 99;\n"
    "	float UISoftMax = 10;\n"
    "	float UIStep = 1;\n"
    "	string UIName = \"Max Lights\";\n"
    "	string UIWidget = \"Slider\";\n"
    "	string UIGroup = \"Lighting\";\n"
    "> = 3;\n"
    "\n"
    "uniform int Light0Type : LIGHTTYPE\n"
    "<\n"
    "	string UIName = \"Light 0 Type\";\n"
    "	float UIMin = 0;\n"
    "	float UIMax = 5;\n"
    "	float UIStep = 1;\n"
    "	string UIWidget = \"None\";\n"
    "	string Object = \"Light 0\";\n"
    "> = 3;\n"
    "\n"
    "uniform int Light1Type : LIGHTTYPE\n"
    "<\n"
    "	string UIName = \"Light 1 Type\";\n"
    "	float UIMin = 0;\n"
    "	float UIMax = 5;\n"
    "	float UIStep = 1;\n"
    "	string UIWidget = \"None\";\n"
    "	string Object = \"Light 1\";\n"
    "> = 3;\n"
    "\n"
    "uniform int Light2Type : LIGHTTYPE\n"
    "<\n"
    "	string UIName = \"Light 2 Type\";\n"
    "	float UIMin = 0;\n"
    "	float UIMax = 5;\n"
    "	float UIStep = 1;\n"
    "	string UIWidget = \"None\";\n"
    "	string Object = \"Light 2\";\n"
    "> = 3;\n"
    "\n"
    "uniform vec3 Light0Color\n"
    "<\n"
    "	string UIName = \"Light 0 Color\";\n"
    "	string UIWidget = \"ColorPicker\";\n"
    "	string Object = \"Light 0\";\n"
    "> = {1.0, 1.0, 1.0};\n"
    "\n"
    "uniform vec3 Light1Color\n"
    "<\n"
    "	string UIName = \"Light 1 Color\";\n"
    "	string UIWidget = \"ColorPicker\";\n"
    "	string Object = \"Light 1\";\n"
    "> = {1.0, 1.0, 1.0};\n"
    "\n"
    "uniform vec3 Light2Color\n"
    "<\n"
    "	string UIName = \"Light 2 Color\";\n"
    "	string UIWidget = \"ColorPicker\";\n"
    "	string Object = \"Light 2\";\n"
    "> = {1.0, 1.0, 1.0};\n"
    "\n"
    "uniform float Light0Intensity : LIGHTINTENSITY\n"
    "<\n"
    "	string UIName = \"Light 0 Intensity\";\n"
    "	float UIMin = 0;\n"
    "	float UIStep = 0.1;\n"
    "	string Object = \"Light 0\";\n"
    "> = 1.0;\n"
    "\n"
    "uniform float Light1Intensity : LIGHTINTENSITY\n"
    "<\n"
    "	string UIName = \"Light 1 Intensity\";\n"
    "	float UIMin = 0;\n"
    "	float UIStep = 0.1;\n"
    "	string Object = \"Light 1\";\n"
    "> = 1.0;\n"
    "\n"
    "uniform float Light2Intensity : LIGHTINTENSITY\n"
    "<\n"
    "	string UIName = \"Light 2 Intensity\";\n"
    "	float UIMin = 0;\n"
    "	float UIStep = 0.1;\n"
    "	string Object = \"Light 2\";\n"
    "> = 1.0;\n"
    "\n"
    "uniform vec3 Light0Pos : POSITION\n"
    "<\n"
    "	string UIName = \"Light 0 Position\";\n"
    "	string Space = \"World\";\n"
    "	string Object = \"Light 0\";\n"
    "> = {1.0, 1.0, 1.0};\n"
    "\n"
    "uniform vec3 Light1Pos : POSITION\n"
    "<\n"
    "	string UIName = \"Light 1 Position\";\n"
    "	string Space = \"World\";\n"
    "	string Object = \"Light 1\";\n"
    "> = { -1.0, -1.0, 1.0 };\n"
    "\n"
    "uniform vec3 Light2Pos : POSITION\n"
    "<\n"
    "	string UIName = \"Light 2 Position\";\n"
    "	string Space = \"World\";\n"
    "	string Object = \"Light 2\";\n"
    "> = { 1.0, -1.0, -1.0 };\n"
    "\n"
    "uniform vec3 Light0Dir : DIRECTION\n"
    "<\n"
    "	string UIName = \"Light 0 Direction\";\n"
    "	string Space = \"World\";\n"
    "	string Object = \"Light 0\";\n"
    "> = {-1.0, -1.0, 0.0};\n"
    "\n"
    "uniform vec3 Light1Dir : DIRECTION\n"
    "<\n"
    "	string UIName = \"Light 1 Direction\";\n"
    "	string Space = \"World\";\n"
    "	string Object = \"Light 1\";\n"
    "> = {-1.0, 1.0, 0.0};\n"
    "\n"
    "uniform vec3 Light2Dir : DIRECTION\n"
    "<\n"
    "	string UIName = \"Light 2 Direction\";\n"
    "	string Space = \"World\";\n"
    "	string Object = \"Light 2\";\n"
    "> = {0.0, -1.0, 1.0};\n"
    "\n"
    "uniform float Light0Attenuation : DECAYRATE\n"
    "<\n"
    "	string UIName = \"Light 0 Decay\";\n"
    "	float UIMin = 0;\n"
    "	float UIStep = 1;\n"
    "	string Object = \"Light 0\";\n"
    "> = 0.01;\n"
    "\n"
    "uniform float Light1Attenuation : DECAYRATE\n"
    "<\n"
    "	string UIName = \"Light 1 Decay\";\n"
    "	float UIMin = 0;\n"
    "	float UIStep = 1;\n"
    "	string Object = \"Light 1\";\n"
    "> = 0.01;\n"
    "\n"
    "uniform float Light2Attenuation : DECAYRATE\n"
    "<\n"
    "	string UIName = \"Light 2 Decay\";\n"
    "	float UIMin = 0;\n"
    "	float UIStep = 1;\n"
    "	string Object = \"Light 2\";\n"
    "> = 0.01;\n"
    "\n"
    "uniform float Light0ConeAngle : HOTSPOT\n"
    "<\n"
    "	string UIName = \"Light 0 Cone Angle\";\n"
    "	float UIMin = 0;\n"
    "	float UIMax = 1.571;\n"
    "	float UIStep = 0.05;\n"
    "	string Space = \"World\";\n"
    "	string Object = \"Light 0\";\n"
    "> = 0.46;\n"
    "\n"
    "uniform float Light1ConeAngle : HOTSPOT\n"
    "<\n"
    "	string UIName = \"Light 1 Cone Angle\";\n"
    "	float UIMin = 0;\n"
    "	float UIMax = 1.571;\n"
    "	float UIStep = 0.05;\n"
    "	string Space = \"World\";\n"
    "	string Object = \"Light 1\";\n"
    "> = 0.46;\n"
    "\n"
    "uniform float Light2ConeAngle : HOTSPOT\n"
    "<\n"
    "	string UIName = \"Light 2 Cone Angle\";\n"
    "	float UIMin = 0;\n"
    "	float UIMax = 1.571;\n"
    "	float UIStep = 0.05;\n"
    "	string Space = \"World\";\n"
    "	string Object = \"Light 2\";\n"
    "> = 0.46;\n"
    "\n"
    "uniform float Light0Falloff : FALLOFF\n"
    "<\n"
    "	string UIName = \"Light 0 Penumbra Angle\";\n"
    "	float UIMin = 0;\n"
    "	float UIMax = 1.571;\n"
    "	float UIStep = 0.05;\n"
    "	string Object = \"Light 0\";\n"
    "> = 0.7;\n"
    "\n"
    "uniform float Light1Falloff : FALLOFF\n"
    "<\n"
    "	string UIName = \"Light 1 Penumbra Angle\";\n"
    "	float UIMin = 0;\n"
    "	float UIMax = 1.571;\n"
    "	float UIStep = 0.05;\n"
    "	string Object = \"Light 1\";\n"
    "> = 0.7;\n"
    "\n"
    "uniform float Light2Falloff : FALLOFF\n"
    "<\n"
    "	string UIName = \"Light 2 Penumbra Angle\";\n"
    "	float UIMin = 0;\n"
    "	float UIMax = 1.571;\n"
    "	float UIStep = 0.05;\n"
    "	string Object = \"Light 2\";\n"
    "> = 0.7;\n"
    "\n"
    "uniform bool Light0ShadowOn : SHADOWFLAG\n"
    "<\n"
    "	string UIName = \"Light 0 Casts Shadow\";\n"
    "	string Object = \"Light 0\";\n"
    "> = false;\n"
    "\n"
    "uniform bool Light1ShadowOn : SHADOWFLAG\n"
    "<\n"
    "	string UIName = \"Light 1 Casts Shadow\";\n"
    "	string Object = \"Light 1\";\n"
    "> = false;\n"
    "\n"
    "uniform bool Light2ShadowOn : SHADOWFLAG\n"
    "<\n"
    "	string UIName = \"Light 2 Casts Shadow\";\n"
    "	string Object = \"Light 2\";\n"
    "> = false;\n"
    "\n"
    "uniform mat4 Light0ViewPrj : SHADOWMAPMATRIX\n"
    "<\n"
    "	string Object = \"Light 0\";\n"
    "	string UIName = \"Light 0 Matrix\";\n"
    "	string UIWidget = \"None\";\n"
    ">;\n"
    "\n"
    "uniform mat4 Light1ViewPrj : SHADOWMAPMATRIX\n"
    "<\n"
    "	string Object = \"Light 1\";\n"
    "	string UIName = \"Light 1 Matrix\";\n"
    "	string UIWidget = \"None\";\n"
    ">;\n"
    "\n"
    "uniform mat4 Light2ViewPrj : SHADOWMAPMATRIX\n"
    "<\n"
    "	string Object = \"Light 2\";\n"
    "	string UIName = \"Light 2 Matrix\";\n"
    "	string UIWidget = \"None\";\n"
    ">;\n"
    "\n"
    "uniform vec3 Light0ShadowColor : SHADOWCOLOR\n"
    "<\n"
    "	string UIName = \"Light 0 Shadow Color\";\n"
    "	string Object = \"Light 0\";\n"
    "> = {0, 0, 0};\n"
    "\n"
    "uniform vec3 Light1ShadowColor : SHADOWCOLOR\n"
    "<\n"
    "	string UIName = \"Light 1 Shadow Color\";\n"
    "	string Object = \"Light 1\";\n"
    "> = {0, 0, 0};\n"
    "\n"
    "uniform vec3 Light2ShadowColor : SHADOWCOLOR\n"
    "<\n"
    "	string UIName = \"Light 2 Shadow Color\";\n"
    "	string Object = \"Light 2\";\n"
    "> = {0, 0, 0};\n"
    "\n"
    "uniform texture2D IBL_file_texture : SourceTexture;\n"
    "uniform sampler2D IBL_file = sampler_state\n"
    "{\n"
    "    Texture = <IBL_file_texture>;\n"
    "};\n"
    "\n"
    "\n"
    "// ----------------------------------- Light Functions --------------------------------------\n"
    "\n"
    "#define M_PI 3.1415926535897932384626433832795\n"
    "\n"
    "GLSLShader PixelShader_LightFuncs\n"
    "{\n"
    "	int GetLightType(int ActiveLightIndex) \n"
    "	{ \n"
    "		if (ActiveLightIndex == 0) \n"
    "			return Light0Type; \n"
    "		else if (ActiveLightIndex == 1) \n"
    "			return Light1Type; \n"
    "		else \n"
    "			return Light2Type; \n"
    "	}\n"
    "\n"
    "	vec3 GetLightColor(int ActiveLightIndex) \n"
    "	{ \n"
    "		if (ActiveLightIndex == 0) \n"
    "			return Light0Color; \n"
    "		else if (ActiveLightIndex == 1) \n"
    "			return Light1Color; \n"
    "		else \n"
    "			return Light2Color; \n"
    "	}\n"
    "\n"
    "	float GetLightIntensity(int ActiveLightIndex) \n"
    "	{ \n"
    "		if (ActiveLightIndex == 0) \n"
    "			return Light0Intensity; \n"
    "		else if (ActiveLightIndex == 1) \n"
    "			return Light1Intensity; \n"
    "		else \n"
    "			return Light2Intensity; \n"
    "	}\n"
    "\n"
    "	vec3 GetLightPos(int ActiveLightIndex) \n"
    "	{ \n"
    "		if (ActiveLightIndex == 0) \n"
    "			return Light0Pos; \n"
    "		else if (ActiveLightIndex == 1) \n"
    "			return Light1Pos; \n"
    "		else \n"
    "			return Light2Pos; \n"
    "	}\n"
    "\n"
    "	vec3 GetLightDir(int ActiveLightIndex) \n"
    "	{ \n"
    "		if (ActiveLightIndex == 0) \n"
    "			return normalize(Light0Dir); \n"
    "		else if (ActiveLightIndex == 1) \n"
    "			return normalize(Light1Dir); \n"
    "		else \n"
    "			return normalize(Light2Dir); \n"
    "	}\n"
    "\n"
    "	float GetLightAttenuation(int ActiveLightIndex) \n"
    "	{ \n"
    "		if (ActiveLightIndex == 0) \n"
    "			return Light0Attenuation; \n"
    "		else if (ActiveLightIndex == 1) \n"
    "			return Light1Attenuation; \n"
    "		else \n"
    "			return Light2Attenuation; \n"
    "	}\n"
    "\n"
    "	float GetLightConeAngle(int ActiveLightIndex) \n"
    "	{ \n"
    "		if (ActiveLightIndex == 0) \n"
    "			return Light0ConeAngle; \n"
    "		else if (ActiveLightIndex == 1) \n"
    "			return Light1ConeAngle; \n"
    "		else \n"
    "			return Light2ConeAngle; \n"
    "	}\n"
    "\n"
    "	float GetLightFalloff(int ActiveLightIndex) \n"
    "	{ \n"
    "		if (ActiveLightIndex == 0) \n"
    "			return Light0Falloff; \n"
    "		else if (ActiveLightIndex == 1) \n"
    "			return Light1Falloff; \n"
    "		else \n"
    "			return Light2Falloff; \n"
    "	}\n"
    "\n"
    "	bool GetLightShadowOn(int ActiveLightIndex) \n"
    "	{ \n"
    "		if (ActiveLightIndex == 0) \n"
    "			return Light0ShadowOn; \n"
    "		else if (ActiveLightIndex == 1) \n"
    "			return Light1ShadowOn; \n"
    "		else \n"
    "			return Light2ShadowOn; \n"
    "	}\n"
    "\n"
    "	mat4 GetLightViewPrj(int ActiveLightIndex) \n"
    "	{ \n"
    "		if (ActiveLightIndex == 0) \n"
    "			return Light0ViewPrj; \n"
    "		else if (ActiveLightIndex == 1) \n"
    "			return Light1ViewPrj; \n"
    "		else \n"
    "			return Light2ViewPrj; \n"
    "	}\n"
    "\n"
    "	vec3 GetLightShadowColor(int ActiveLightIndex) \n"
    "	{ \n"
    "		if (ActiveLightIndex == 0) \n"
    "			return Light0ShadowColor; \n"
    "		else if (ActiveLightIndex == 1) \n"
    "			return Light1ShadowColor; \n"
    "		else \n"
    "			return Light2ShadowColor; \n"
    "	}\n"
    "\n"
    "	vec3 GetLightVectorFunction(int ActiveLightIndex, vec3 LightPosition, vec3 VertexWorldPosition, vec3 LightDirection)\n"
    "	{\n"
    "		int _LightType = GetLightType(ActiveLightIndex);\n"
    "		bool IsDirectionalLight = (_LightType == 4);\n"
    "		vec3 LerpOp = mix((LightPosition - VertexWorldPosition), -(LightDirection), float(IsDirectionalLight));\n"
    "		return LerpOp;\n"
    "	}\n"
    "\n"
    "	float LightDecayFunction(int ActiveLightIndex, vec3 LightVectorUN, float Attenuation)\n"
    "	{\n"
    "		float DecayContribution116 = 1.0;\n"
    "		if (Attenuation > 0.001)\n"
    "		{\n"
    "			float PowOp = pow(length(LightVectorUN), Attenuation);\n"
    "			float DivOp = (1.0 / PowOp);\n"
    "			DecayContribution116 = DivOp;\n"
    "		}\n"
    "		return DecayContribution116;\n"
    "	}\n"
    "\n"
    "	float LightConeAngleFunction(int ActiveLightIndex, vec3 LightVector, vec3 LightDirection, float ConeAngle, float ConeFalloff)\n"
    "	{\n"
    "		float CosOp = cos(max(ConeFalloff, ConeAngle));\n"
    "		float DotOp = dot(LightVector, -(LightDirection));\n"
    "		float SmoothStepOp = smoothstep(CosOp, cos(ConeAngle), DotOp);\n"
    "		return SmoothStepOp;\n"
    "	}\n"
    "\n"
    "	vec3 LightContributionFunction(int ActiveLightIndex, vec3 VertexWorldPosition, vec3 LightVectorUN)\n"
    "	{\n"
    "		float _LightIntensity = GetLightIntensity(ActiveLightIndex);\n"
    "		int _LightType = GetLightType(ActiveLightIndex);\n"
    "		bool IsDirectionalLight = (_LightType == 4);\n"
    "		float DecayMul162 = 1.0;\n"
    "		if (!IsDirectionalLight)\n"
    "		{\n"
    "			float _LightAttenuation = GetLightAttenuation(ActiveLightIndex);\n"
    "			DecayMul162 = LightDecayFunction(ActiveLightIndex, LightVectorUN, _LightAttenuation);\n"
    "		}\n"
    "		bool IsSpotLight = (_LightType == 2);\n"
    "		float ConeMul164 = 1.0;\n"
    "		if (IsSpotLight)\n"
    "		{\n"
    "			vec3 NormOp = normalize(LightVectorUN);\n"
    "			vec3 _LightDir = GetLightDir(ActiveLightIndex);\n"
    "			float _LightConeAngle = GetLightConeAngle(ActiveLightIndex);\n"
    "			float _LightFalloff = GetLightFalloff(ActiveLightIndex);\n"
    "			ConeMul164 = LightConeAngleFunction(ActiveLightIndex, NormOp, _LightDir, _LightConeAngle, _LightFalloff);\n"
    "		}\n"
    "		float ShadowMul165 = 1.0;\n"
    "//		bool _LightShadowOn = GetLightShadowOn(ActiveLightIndex);\n"
    "//		if (_LightShadowOn)\n"
    "//		{\n"
    "//			mat4 _LightViewPrj = GetLightViewPrj(ActiveLightIndex);\n"
    "//			ShadowMapOutput ShadowMap178 = ShadowMapFunction(ActiveLightIndex, _LightViewPrj, 0.01, VertexWorldPosition);\n"
    "//			vec3 _LightShadowColor = GetLightShadowColor(ActiveLightIndex);\n"
    "//			float ShadowColorMix = mix(ShadowMap178.LightGain, 1.0, float(_LightShadowColor.x));\n"
    "//			ShadowMul165 = ShadowColorMix;\n"
    "//		}\n"
    "		float DecayShadowConeMul = ((ShadowMul165 * ConeMul164) * DecayMul162);\n"
    "		vec3 _LightColor = GetLightColor(ActiveLightIndex);\n"
    "		vec3 result = ((_LightColor * DecayShadowConeMul) * _LightIntensity);\n"
    "		return result;\n"
    "	}\n"
    "\n"
    "vec2 GetSphericalCoords(vec3 vec)\n"
    "{\n"
    "    float v = acos(clamp(vec.z, -1.0, 1.0));\n"
    "    float u = clamp(vec.x / sin(v), -1.0, 1.0);\n"
    "    if (vec.y >= 0.0)\n"
    "        u = acos(u);\n"
    "    else\n"
    "        u = 2 * M_PI - acos(u);\n"
    "    return vec2(u / (2 * M_PI), v / M_PI);\n"
    "}\n"
    "\n"
    "vec3 EnvironmentLight(vec3 normal, vec3 view, float rougness)\n"
    "{\n"
    "    if (textureSize(IBL_file, 0).x > 0)\n"
    "    {\n"
    "        vec3 dir = reflect(-view, normal);\n"
    "        // Y is up vector\n"
    "        dir = vec3(dir.x, -dir.z, dir.y);\n"
    "        vec2 uv = GetSphericalCoords(dir);\n"
    "        float lod = (1.0 - 0.1 / (rougness + 0.1)) * 10.0;\n"
    "        return textureLod(IBL_file, uv, lod).rgb;\n"
    "    }\n"
    "    return vec3(0.0); \n"
    "}\n"
    "\n"
    "	float FresnelSchlickTIR(float nt, float ni, vec3 n, vec3 i)\n"
    "	{\n"
    "		float R0 = (nt - ni) / (nt + ni);\n"
    "		R0 *= R0;\n"
    "		float CosX = dot(n, i);\n"
    "		if (ni > nt)\n"
    "		{\n"
    "			float inv_eta = ni / nt;\n"
    "			float SinT2 = inv_eta * inv_eta * (1.0f - CosX * CosX);\n"
    "			if (SinT2 > 1.0f) {\n"
    "				return 1.0f; // TIR\n"
    "			}\n"
    "			CosX = sqrt(1.0f - SinT2);\n"
    "		}\n"
    "		return R0 + (1.0f - R0) * pow(1.0 - CosX, 5.0);\n"
    "	}\n"
    "}\n"
    "\n";

const char* kShaderBlock_VertexShader =
    "// -----------------------------------------------------------------------------------------\n"
    "\n"
    "// Data from application to vertex buffer\n"
    "attribute AppData\n"
    "{\n"
    "	vec3 inPosition : POSITION;\n"
    "	vec3 inNormal   : NORMAL;\n"
    "	vec3 inTangent  : TANGENT;\n"
    "	vec3 inBinormal : BINORMAL;\n"
    "	vec2 inUV       : TEXCOORD0;\n"
    "};\n"
    "\n"
    "// Data passed from vertex shader to pixel shader\n"
    "attribute VertexOutput\n"
    "{\n"
    "	vec3 WorldPosition : POSITION;\n"
    "	vec3 WorldNormal   : NORMAL;\n"
    "	vec3 WorldTangent  : TANGENT;\n"
    "	vec3 WorldBinormal : BINORMAL;\n"
    "	vec2 UV0           : TEXCOORD1;\n"
    "	vec3 WorldView     : TEXCOORD2;\n"
    "	float FrontFacing  : FACE;\n"
    "};\n"
    "\n"
    "// Vertex shader\n"
    "GLSLShader VS\n"
    "{\n"
    "	void main()\n"
    "	{\n"
    "		vec4 Po = vec4(inPosition.xyz,1);\n"
    "		vec4 Pw = gWorldXf * Po;\n"
    "		VS_OUT.WorldPosition = Pw.xyz;\n"
    "		VS_OUT.WorldView     = normalize(gViewIXf[3].xyz - Pw.xyz);\n"
    "		VS_OUT.WorldNormal   = normalize((gWorldITXf * vec4(inNormal,0)).xyz);\n"
    "		VS_OUT.WorldTangent  = normalize((gWorldITXf * vec4(inTangent,0)).xyz);\n"
    "		VS_OUT.WorldBinormal = normalize((gWorldITXf * vec4(inBinormal,0)).xyz);\n"
    "		VS_OUT.UV0 = inUV;\n"
    "		VS_OUT.FrontFacing = dot(VS_OUT.WorldNormal, VS_OUT.WorldView);\n"
    "		vec4 hpos = gWvpXf * Po;\n"
    "		gl_Position = hpos;\n"
    "	}\n"
    "}\n"
    "\n";

const char* kShaderBlock_TechniquesLighting =
    "// Techniques\n"
    "technique Main\n"
    "{\n"
    "	pass p0\n"
    "	{\n"
    "		VertexShader(in AppData, out VertexOutput VS_OUT) = VS;\n"
    "		PixelShader(in VertexOutput PS_IN, out PixelOutput) = { PixelShader_LightFuncs, PS };\n"
    "	}\n"
    "}\n"
    "\n";

const char* kShaderBlock_TechniquesNonLighting =
    "// Techniques\n"
    "technique Main\n"
    "{\n"
    "	pass p0\n"
    "	{\n"
    "		VertexShader(in AppData, out VertexOutput VS_OUT) = VS;\n"
    "		PixelShader(in VertexOutput PS_IN, out PixelOutput) = PS;\n"
    "	}\n"
    "}\n"
    "\n";

} // anonymous namespace

namespace MaterialX
{

DEFINE_SHADER_GENERATOR(OgsFxShaderGenerator, "glsl", "ogsfx")

ShaderPtr OgsFxShaderGenerator::generate(const string& shaderName, ElementPtr element)
{
    ShaderPtr shaderPtr = std::make_shared<Shader>(shaderName);
    shaderPtr->initialize(element, getLanguage(), getTarget());

    Shader& shader = *shaderPtr;

    addExtraShaderUniforms(shader);

    shader.addStr(kShaderBlock_Matrices);

    // Emit all shader uniforms
    for (const Shader::Uniform& uniform : shader.getUniforms())
    {
        emitUniform(
            uniform.first, 
            uniform.second->getType(), 
            uniform.second->getValue(),
            shader
        );
    }
    // Emit all shader varyings
    for (const Shader::Varying& varying : shader.getVaryings())
    {
        emitUniform(
            varying.first,
            varying.second->getType(),
            varying.second->getValue(),
            shader
        );
    }

    const string& outputType = shader.getOutput()->getType();
    if (outputType == kSURFACE)
    {
        shader.addBlock(kShaderBlock_Lights);
    }
    shader.addBlock(kShaderBlock_VertexShader);

    emitTypeDefs(shader);

    // Emit shader output
    string type;
    if (outputType == "float")
    {
        // Hack to avoid float output from OgsFX shader
        // Gives random artifacts in LookdevX viewer
        type = "vec3";
    }
    else
    {
        type = _syntax->getOutputTypeName(outputType);
        size_t pos = type.find("out ");
        if (pos != string::npos)
        {
            // Strip the 'out' decorator
            type.erase(pos, 4);
        }
    }
    const string variable = _syntax->getVariableName(*shader.getOutput());
    shader.addLine("// Data output by the pixel shader", false);
    shader.addLine("attribute PixelOutput", false);
    shader.beginScope(Shader::Brackets::BRACES);
    shader.addLine(type + " " + variable);
    shader.endScope(true);
    shader.newLine();

    // Pixel shader
    shader.addStr("GLSLShader PS\n");
    shader.beginScope(Shader::Brackets::BRACES);

    emitFunctions(shader);

    shader.addLine("void main()", false);
    shader.beginScope(Shader::Brackets::BRACES);
    emitShaderBody(shader);
    shader.endScope();

    shader.endScope();
    shader.newLine();

    if (outputType == kSURFACE)
    {
        shader.addBlock(kShaderBlock_TechniquesLighting);
    }
    else
    {
        shader.addBlock(kShaderBlock_TechniquesNonLighting);
    }

    // Release resources used by shader gen
    shaderPtr->finalize();

    return shaderPtr;
}

// TODO: Remove the hard coded gamma transform
void OgsFxShaderGenerator::emitFinalOutput(Shader& shader) const
{
    const OutputPtr& output = shader.getOutput();
    const NodePtr connectedNode = output->getConnectedNode();

    string finalResult = _syntax->getVariableName(*connectedNode);

    const string& outputType = output->getType();
    if (outputType == kSURFACE)
    {
        finalResult = finalResult + ".bsdf + " + finalResult + ".edf";
        shader.addLine(_syntax->getVariableName(*output) + " = vec4(" + finalResult + ", 1.0)");
    }
    else
    {
        if (output->getChannels() != EMPTY_STRING)
        {
            finalResult = _syntax->getSwizzledVariable(finalResult, output->getType(), connectedNode->getType(), output->getChannels());
        }
        if (outputType == "float")
        {
            // Hack to avoid float output from OgsFX shader
            // Gives random artifacts in LookdevX viewer
            finalResult = "vec3(" + finalResult + ", " + finalResult + ", " + finalResult + ")";
        }
        shader.addLine(_syntax->getVariableName(*output) + " = " + finalResult);
    }
}

void OgsFxShaderGenerator::emitUniform(const string& name, const string& type, const ValuePtr& value, Shader& shader)
{
    // A file texture input needs special handling on GLSL
    if (type == kFilename)
    {
        std::stringstream str;
        str << "uniform texture2D " << name << "_texture : SourceTexture;\n";
        str << "uniform sampler2D " << name << " = sampler_state\n";
        str << "{\n    Texture = <" << name << "_texture>;\n};\n";
        shader.addBlock(str.str());
    }
    else
    {
        shader.beginLine();
        shader.addStr("uniform ");
        ShaderGenerator::emitUniform(name, type, value, shader);
        shader.endLine();
    }
}

void OgsFxShaderGenerator::emitInput(const ValueElement& port, Shader& shader)
{
    if (port.isA<Parameter>() && 
        useAsShaderUniform(*port.asA<Parameter>()))
    {
        // This input is promoted to a shader input
        // So just use the name of that shader input
        shader.addStr(_syntax->getVariableName(port));
    }
    else
    {
        GlslShaderGenerator::emitInput(port, shader);
    }
}

void OgsFxShaderGenerator::addExtraShaderUniforms(Shader& shader)
{
    // Run over all node ports and check if any should be promoted to shader inputs
    // (this is the case for file texture filename inputs)
    for (const SgNode& node : shader.getNodes())
    {
        for (ParameterPtr param : node.getNode().getParameters())
        {
            if (useAsShaderUniform(*param))
            {
                const string name = _syntax->getVariableName(*param);
                shader.addUniform(Shader::Uniform(name, param));
            }
        }
    }
}

bool OgsFxShaderGenerator::useAsShaderUniform(const Parameter& param) const
{
    // Unconnected file texture inputs should be promoted to shader uniforms
    if (param.getType() == kFilename && param.getInterfaceName().empty())
    {
        ConstElementPtr parent = param.getParent();
        const string& nodeName = parent->isA<NodeDef>() ?
            parent->asA<NodeDef>()->getNode() : parent->asA<Node>()->getCategory();
        return nodeName == "image";
    }
    return false;
}

} // namespace MaterialX
