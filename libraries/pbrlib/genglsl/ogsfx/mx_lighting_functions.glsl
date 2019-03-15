// --------------------------------- OgsFx Lighting Functions --------------------------------------

int mx_getLightType(int index)
{
    if (index == 0)
        return u_light0Type;
    else if (index == 1)
        return u_light1Type;
    else if (index == 2)
        return u_light2Type;
    else if (index == 3)
        return u_light3Type;
    else if (index == 4)
        return u_light4Type;
    else if (index == 5)
        return u_light5Type;
    else if (index == 6)
        return u_light6Type;
    else
        return u_light7Type;
}

vec3 mx_getLightColor(int index)
{
    if (index == 0)
        return u_light0Color;
    else if (index == 1)
        return u_light1Color;
    else if (index == 2)
        return u_light2Color;
    else if (index == 3)
        return u_light3Color;
    else if (index == 4)
        return u_light4Color;
    else if (index == 5)
        return u_light5Color;
    else if (index == 6)
        return u_light6Color;
    else
        return u_light7Color;
}

float mx_getLightIntensity(int index)
{
    if (index == 0)
        return u_light0Intensity;
    else if (index == 1)
        return u_light1Intensity;
    else if (index == 2)
        return u_light2Intensity;
    else if (index == 3)
        return u_light3Intensity;
    else if (index == 4)
        return u_light4Intensity;
    else if (index == 5)
        return u_light5Intensity;
    else if (index == 6)
        return u_light6Intensity;
    else
        return u_light7Intensity;
}

vec3 mx_getLightPos(int index)
{
    if (index == 0)
        return u_light0Pos;
    else if (index == 1)
        return u_light1Pos;
    else if (index == 2)
        return u_light2Pos;
    else if (index == 3)
        return u_light3Pos;
    else if (index == 4)
        return u_light4Pos;
    else if (index == 5)
        return u_light5Pos;
    else if (index == 6)
        return u_light6Pos;
    else
        return u_light7Pos;
}

vec3 mx_getLightDir(int index)
{
    if (index == 0)
        return u_light0Dir;
    else if (index == 1)
        return u_light1Dir;
    else if (index == 2)
        return u_light2Dir;
    else if (index == 3)
        return u_light3Dir;
    else if (index == 4)
        return u_light4Dir;
    else if (index == 5)
        return u_light5Dir;
    else if (index == 6)
        return u_light6Dir;
    else
        return u_light7Dir;
}

float mx_getLightDecayRate(int index)
{
    if (index == 0)
        return u_light0DecayRate;
    else if (index == 1)
        return u_light1DecayRate;
    else if (index == 2)
        return u_light2DecayRate;
    else if (index == 3)
        return u_light3DecayRate;
    else if (index == 4)
        return u_light4DecayRate;
    else if (index == 5)
        return u_light5DecayRate;
    else if (index == 6)
        return u_light6DecayRate;
    else
        return u_light7DecayRate;
}

float mx_getLightConeAngle(int index)
{
    if (index == 0)
        return cos(u_light0ConeAngle);
    else if (index == 1)
        return cos(u_light1ConeAngle);
    else if (index == 2)
        return cos(u_light2ConeAngle);
    else if (index == 3)
        return cos(u_light3ConeAngle);
    else if (index == 4)
        return cos(u_light4ConeAngle);
    else if (index == 5)
        return cos(u_light5ConeAngle);
    else if (index == 6)
        return cos(u_light6ConeAngle);
    else
        return cos(u_light7ConeAngle);
}

float mx_getLightPenumbraAngle(int index)
{
    if (index == 0)
        return cos(u_light0PenumbraAngle);
    else if (index == 1)
        return cos(u_light1PenumbraAngle);
    else if (index == 2)
        return cos(u_light2PenumbraAngle);
    else if (index == 3)
        return cos(u_light3PenumbraAngle);
    else if (index == 4)
        return cos(u_light4PenumbraAngle);
    else if (index == 5)
        return cos(u_light5PenumbraAngle);
    else if (index == 6)
        return cos(u_light6PenumbraAngle);
    else
        return cos(u_light7PenumbraAngle);
}
