// --------------------------------- OgsFx Lighting Functions --------------------------------------

int getLightType(int index)
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
    else if (index == 7)
        return u_light7Type;
    else if (index == 8)
        return u_light8Type;
    else if (index == 9)
        return u_light9Type;
    else if (index == 10)
        return u_light10Type;
    else if (index == 11)
        return u_light11Type;
    else if (index == 12)
        return u_light12Type;
    else if (index == 13)
        return u_light13Type;
    else if (index == 14)
        return u_light14Type;
    else
        return u_light15Type;
}

vec3 getLightColor(int index) 
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
    else if (index == 7)
        return u_light7Color;
    else if (index == 8)
        return u_light8Color;
    else if (index == 9)
        return u_light9Color;
    else if (index == 10)
        return u_light10Color;
    else if (index == 11)
        return u_light11Color;
    else if (index == 12)
        return u_light12Color;
    else if (index == 13)
        return u_light13Color;
    else if (index == 14)
        return u_light14Color;
    else
        return u_light15Color;
}

float getLightIntensity(int index)
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
    else if (index == 7)
        return u_light7Intensity;
    else if (index == 8)
        return u_light8Intensity;
    else if (index == 9)
        return u_light9Intensity;
    else if (index == 10)
        return u_light10Intensity;
    else if (index == 11)
        return u_light11Intensity;
    else if (index == 12)
        return u_light12Intensity;
    else if (index == 13)
        return u_light13Intensity;
    else if (index == 14)
        return u_light14Intensity;
    else
        return u_light15Intensity;
}

vec3 getLightPos(int index)
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
    else if (index == 7)
        return u_light7Pos;
    else if (index == 8)
        return u_light8Pos;
    else if (index == 9)
        return u_light9Pos;
    else if (index == 10)
        return u_light10Pos;
    else if (index == 11)
        return u_light11Pos;
    else if (index == 12)
        return u_light12Pos;
    else if (index == 13)
        return u_light13Pos;
    else if (index == 14)
        return u_light14Pos;
    else
        return u_light15Pos;
}

vec3 getLightDir(int index) 
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
    else if (index == 7)
        return u_light7Dir;
    else if (index == 8)
        return u_light8Dir;
    else if (index == 9)
        return u_light9Dir;
    else if (index == 10)
        return u_light10Dir;
    else if (index == 11)
        return u_light11Dir;
    else if (index == 12)
        return u_light12Dir;
    else if (index == 13)
        return u_light13Dir;
    else if (index == 14)
        return u_light14Dir;
    else
        return u_light15Dir;
}

float getLightDecayRate(int index)
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
    else if (index == 7)
        return u_light7DecayRate;
    else if (index == 8)
        return u_light8DecayRate;
    else if (index == 9)
        return u_light9DecayRate;
    else if (index == 10)
        return u_light10DecayRate;
    else if (index == 11)
        return u_light11DecayRate;
    else if (index == 12)
        return u_light12DecayRate;
    else if (index == 13)
        return u_light13DecayRate;
    else if (index == 14)
        return u_light14DecayRate;
    else
        return u_light15DecayRate;
}

float getLightConeAngle(int index)
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
    else if (index == 7)
        return cos(u_light7ConeAngle);
    else if (index == 8)
        return cos(u_light8ConeAngle);
    else if (index == 9)
        return cos(u_light9ConeAngle);
    else if (index == 10)
        return cos(u_light10ConeAngle);
    else if (index == 11)
        return cos(u_light11ConeAngle);
    else if (index == 12)
        return cos(u_light12ConeAngle);
    else if (index == 13)
        return cos(u_light13ConeAngle);
    else if (index == 14)
        return cos(u_light14ConeAngle);
    else
        return cos(u_light15ConeAngle);
}

float getLightPenumbraAngle(int index) 
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
    else if (index == 7)
        return cos(u_light7PenumbraAngle);
    else if (index == 8)
        return cos(u_light8PenumbraAngle);
    else if (index == 9)
        return cos(u_light9PenumbraAngle);
    else if (index == 10)
        return cos(u_light10PenumbraAngle);
    else if (index == 11)
        return cos(u_light11PenumbraAngle);
    else if (index == 12)
        return cos(u_light12PenumbraAngle);
    else if (index == 13)
        return cos(u_light13PenumbraAngle);
    else if (index == 14
        return cos(u_light14PenumbraAngle);
    else
        return cos(u_light15PenumbraAngle);
}
