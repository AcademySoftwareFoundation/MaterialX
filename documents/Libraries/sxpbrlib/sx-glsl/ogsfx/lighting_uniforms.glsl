// --------------------------------- OgsFx Lighting Uniforms --------------------------------------

uniform int u_light0Type : LIGHTTYPE
<
    string UIName = "Type";
    float UIMin = 0;
    float UIMax = 5;
    float UIStep = 1;
    string UIWidget = "None";
    string Object = "Light 0";
> = 3;

uniform int u_light1Type : LIGHTTYPE
<
    string UIName = "Type";
    float UIMin = 0;
    float UIMax = 5;
    float UIStep = 1;
    string UIWidget = "None";
    string Object = "Light 1";
> = 3;

uniform int u_light2Type : LIGHTTYPE
<
    string UIName = "Type";
    float UIMin = 0;
    float UIMax = 5;
    float UIStep = 1;
    string UIWidget = "None";
    string Object = "Light 2";
> = 3;

uniform vec3 u_light0Color
<
    string UIName = "Color";
    string UIWidget = "ColorPicker";
    string Object = "Light 0";
> = {1.0, 1.0, 1.0};

uniform vec3 u_light1Color
<
    string UIName = "Color";
    string UIWidget = "ColorPicker";
    string Object = "Light 1";
> = {1.0, 1.0, 1.0};

uniform vec3 u_light2Color
<
    string UIName = "Color";
    string UIWidget = "ColorPicker";
    string Object = "Light 2";
> = {1.0, 1.0, 1.0};

uniform float u_light0Intensity : LIGHTINTENSITY
<
    string UIName = "Intensity";
    float UIMin = 0;
    float UIStep = 0.1;
    string Object = "Light 0";
> = 1.0;

uniform float u_light1Intensity : LIGHTINTENSITY
<
    string UIName = "Intensity";
    float UIMin = 0;
    float UIStep = 0.1;
    string Object = "Light 1";
> = 1.0;

uniform float u_light2Intensity : LIGHTINTENSITY
<
    string UIName = "Intensity";
    float UIMin = 0;
    float UIStep = 0.1;
    string Object = "Light 2";
> = 1.0;

uniform vec3 u_light0Pos : POSITION
<
    string UIName = "Position";
    string Space = "World";
    string Object = "Light 0";
> = {1.0, 1.0, 1.0};

uniform vec3 u_light1Pos : POSITION
<
    string UIName = "Position";
    string Space = "World";
    string Object = "Light 1";
> = { -1.0, -1.0, 1.0 };

uniform vec3 u_light2Pos : POSITION
<
    string UIName = "Position";
    string Space = "World";
    string Object = "Light 2";
> = { 1.0, -1.0, -1.0 };

uniform vec3 u_light0Dir : DIRECTION
<
    string UIName = "Direction";
    string Space = "World";
    string Object = "Light 0";
> = {-1.0, -1.0, 0.0};

uniform vec3 u_light1Dir : DIRECTION
<
    string UIName = "Direction";
    string Space = "World";
    string Object = "Light 1";
> = {-1.0, 1.0, 0.0};

uniform vec3 u_light2Dir : DIRECTION
<
    string UIName = "Direction";
    string Space = "World";
    string Object = "Light 2";
> = {0.0, -1.0, 1.0};

uniform float u_light0DecayRate : DECAYRATE
<
    string UIName = "Decay Rate";
    float UIMin = 0;
    float UIStep = 1;
    string Object = "Light 0";
> = 0.01;

uniform float u_light1DecayRate : DECAYRATE
<
    string UIName = "Decay Rate";
    float UIMin = 0;
    float UIStep = 1;
    string Object = "Light 1";
> = 0.01;

uniform float u_light2DecayRate : DECAYRATE
<
    string UIName = "Decay Rate";
    float UIMin = 0;
    float UIStep = 1;
    string Object = "Light 2";
> = 0.01;

uniform float u_light0ConeAngle : HOTSPOT
<
    string UIName = "Cone Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Space = "World";
    string Object = "Light 0";
> = 0.46;

uniform float u_light1ConeAngle : HOTSPOT
<
    string UIName = "Cone Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Space = "World";
    string Object = "Light 1";
> = 0.46;

uniform float u_light2ConeAngle : HOTSPOT
<
    string UIName = "Cone Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Space = "World";
    string Object = "Light 2";
> = 0.46;

uniform float u_light0PenumbraAngle : FALLOFF
<
    string UIName = "Penumbra Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Object = "Light 0";
> = 0.7;

uniform float u_light1PenumbraAngle : FALLOFF
<
    string UIName = "Penumbra Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Object = "Light 1";
> = 0.7;

uniform float u_light2PenumbraAngle : FALLOFF
<
    string UIName = "Penumbra Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Object = "Light 2";
> = 0.7;
