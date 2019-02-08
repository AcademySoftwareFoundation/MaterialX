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

uniform int u_light3Type : LIGHTTYPE
<
    string UIName = "Type";
    float UIMin = 0;
    float UIMax = 5;
    float UIStep = 1;
    string UIWidget = "None";
    string Object = "Light 3";
> = 3;

uniform int u_light4Type : LIGHTTYPE
<
    string UIName = "Type";
    float UIMin = 0;
    float UIMax = 5;
    float UIStep = 1;
    string UIWidget = "None";
    string Object = "Light 4";
> = 3;

uniform int u_light5Type : LIGHTTYPE
<
    string UIName = "Type";
    float UIMin = 0;
    float UIMax = 5;
    float UIStep = 1;
    string UIWidget = "None";
    string Object = "Light 5";
> = 3;

uniform int u_light6Type : LIGHTTYPE
<
    string UIName = "Type";
    float UIMin = 0;
    float UIMax = 5;
    float UIStep = 1;
    string UIWidget = "None";
    string Object = "Light 6";
> = 3;

uniform int u_light7Type : LIGHTTYPE
<
    string UIName = "Type";
    float UIMin = 0;
    float UIMax = 5;
    float UIStep = 1;
    string UIWidget = "None";
    string Object = "Light 7";
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

uniform vec3 u_light3Color
<
    string UIName = "Color";
    string UIWidget = "ColorPicker";
    string Object = "Light 3";
> = {1.0, 1.0, 1.0};

uniform vec3 u_light4Color
<
    string UIName = "Color";
    string UIWidget = "ColorPicker";
    string Object = "Light 4";
> = {1.0, 1.0, 1.0};

uniform vec3 u_light5Color
<
    string UIName = "Color";
    string UIWidget = "ColorPicker";
    string Object = "Light 5";
> = {1.0, 1.0, 1.0};

uniform vec3 u_light6Color
<
    string UIName = "Color";
    string UIWidget = "ColorPicker";
    string Object = "Light 6";
> = {1.0, 1.0, 1.0};

uniform vec3 u_light7Color
<
    string UIName = "Color";
    string UIWidget = "ColorPicker";
    string Object = "Light 7";
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

uniform float u_light3Intensity : LIGHTINTENSITY
<
    string UIName = "Intensity";
    float UIMin = 0;
    float UIStep = 0.1;
    string Object = "Light 3";
> = 1.0;

uniform float u_light4Intensity : LIGHTINTENSITY
<
    string UIName = "Intensity";
    float UIMin = 0;
    float UIStep = 0.1;
    string Object = "Light 4";
> = 1.0;

uniform float u_light5Intensity : LIGHTINTENSITY
<
    string UIName = "Intensity";
    float UIMin = 0;
    float UIStep = 0.1;
    string Object = "Light 5";
> = 1.0;

uniform float u_light6Intensity : LIGHTINTENSITY
<
    string UIName = "Intensity";
    float UIMin = 0;
    float UIStep = 0.1;
    string Object = "Light 6";
> = 1.0;

uniform float u_light7Intensity : LIGHTINTENSITY
<
    string UIName = "Intensity";
    float UIMin = 0;
    float UIStep = 0.1;
    string Object = "Light 7";
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

uniform vec3 u_light3Pos : POSITION
<
    string UIName = "Position";
    string Space = "World";
    string Object = "Light 3";
> = { 1.0, -1.0, -1.0 };

uniform vec3 u_light4Pos : POSITION
<
    string UIName = "Position";
    string Space = "World";
    string Object = "Light 4";
> = { 1.0, -1.0, -1.0 };

uniform vec3 u_light5Pos : POSITION
<
    string UIName = "Position";
    string Space = "World";
    string Object = "Light 5";
> = { 1.0, -1.0, -1.0 };

uniform vec3 u_light6Pos : POSITION
<
    string UIName = "Position";
    string Space = "World";
    string Object = "Light 6";
> = { 1.0, -1.0, -1.0 };

uniform vec3 u_light7Pos : POSITION
<
    string UIName = "Position";
    string Space = "World";
    string Object = "Light 7";
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

uniform vec3 u_light3Dir : DIRECTION
<
    string UIName = "Direction";
    string Space = "World";
    string Object = "Light 3";
> = {0.0, -1.0, 1.0};

uniform vec3 u_light4Dir : DIRECTION
<
    string UIName = "Direction";
    string Space = "World";
    string Object = "Light 4";
> = {0.0, -1.0, 1.0};

uniform vec3 u_light5Dir : DIRECTION
<
    string UIName = "Direction";
    string Space = "World";
    string Object = "Light 5";
> = {0.0, -1.0, 1.0};

uniform vec3 u_light6Dir : DIRECTION
<
    string UIName = "Direction";
    string Space = "World";
    string Object = "Light 6";
> = {0.0, -1.0, 1.0};

uniform vec3 u_light7Dir : DIRECTION
<
    string UIName = "Direction";
    string Space = "World";
    string Object = "Light 7";
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

uniform float u_light3DecayRate : DECAYRATE
<
    string UIName = "Decay Rate";
    float UIMin = 0;
    float UIStep = 1;
    string Object = "Light 3";
> = 0.01;

uniform float u_light4DecayRate : DECAYRATE
<
    string UIName = "Decay Rate";
    float UIMin = 0;
    float UIStep = 1;
    string Object = "Light 4";
> = 0.01;

uniform float u_light5DecayRate : DECAYRATE
<
    string UIName = "Decay Rate";
    float UIMin = 0;
    float UIStep = 1;
    string Object = "Light 5";
> = 0.01;

uniform float u_light6DecayRate : DECAYRATE
<
    string UIName = "Decay Rate";
    float UIMin = 0;
    float UIStep = 1;
    string Object = "Light 6";
> = 0.01;

uniform float u_light7DecayRate : DECAYRATE
<
    string UIName = "Decay Rate";
    float UIMin = 0;
    float UIStep = 1;
    string Object = "Light 7";
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

uniform float u_light3ConeAngle : HOTSPOT
<
    string UIName = "Cone Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Space = "World";
    string Object = "Light 3";
> = 0.46;

uniform float u_light4ConeAngle : HOTSPOT
<
    string UIName = "Cone Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Space = "World";
    string Object = "Light 4";
> = 0.46;

uniform float u_light5ConeAngle : HOTSPOT
<
    string UIName = "Cone Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Space = "World";
    string Object = "Light 5";
> = 0.46;

uniform float u_light6ConeAngle : HOTSPOT
<
    string UIName = "Cone Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Space = "World";
    string Object = "Light 6";
> = 0.46;

uniform float u_light7ConeAngle : HOTSPOT
<
    string UIName = "Cone Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Space = "World";
    string Object = "Light 7";
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

uniform float u_light3PenumbraAngle : FALLOFF
<
    string UIName = "Penumbra Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Object = "Light 3";
> = 0.7;

uniform float u_light4PenumbraAngle : FALLOFF
<
    string UIName = "Penumbra Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Object = "Light 4";
> = 0.7;

uniform float u_light5PenumbraAngle : FALLOFF
<
    string UIName = "Penumbra Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Object = "Light 5";
> = 0.7;

uniform float u_light6PenumbraAngle : FALLOFF
<
    string UIName = "Penumbra Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Object = "Light 6";
> = 0.7;

uniform float u_light7PenumbraAngle : FALLOFF
<
    string UIName = "Penumbra Angle";
    float UIMin = 0;
    float UIMax = 1.571;
    float UIStep = 0.05;
    string Object = "Light 7";
> = 0.7;
