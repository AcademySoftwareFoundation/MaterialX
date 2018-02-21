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
