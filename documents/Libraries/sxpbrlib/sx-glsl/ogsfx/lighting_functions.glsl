// --------------------------------- OgsFx Lighting Functions --------------------------------------

int getLightType(int index) 
{ 
    if (index == 0) 
        return u_light0Type; 
    else if (index == 1) 
        return u_light1Type; 
    else 
        return u_light2Type; 
}

vec3 getLightColor(int index) 
{ 
    if (index == 0) 
        return u_light0Color; 
    else if (index == 1) 
        return u_light1Color; 
    else 
        return u_light2Color; 
}

float getLightIntensity(int index) 
{ 
    if (index == 0) 
        return u_light0Intensity; 
    else if (index == 1) 
        return u_light1Intensity; 
    else 
        return u_light2Intensity; 
}

vec3 getLightPos(int index) 
{ 
    if (index == 0) 
        return u_light0Pos; 
    else if (index == 1) 
        return u_light1Pos; 
    else 
        return u_light2Pos; 
}

vec3 getLightDir(int index) 
{ 
    if (index == 0) 
        return u_light0Dir; 
    else if (index == 1) 
        return u_light1Dir; 
    else 
        return u_light2Dir; 
}

float getLightDecayRate(int index) 
{ 
    if (index == 0) 
        return u_light0DecayRate; 
    else if (index == 1) 
        return u_light1DecayRate; 
    else 
        return u_light2DecayRate; 
}

float getLightConeAngle(int index) 
{ 
    if (index == 0) 
        return cos(u_light0ConeAngle);
    else if (index == 1) 
        return cos(u_light1ConeAngle);
    else 
        return cos(u_light2ConeAngle);
}

float getLightPenumbraAngle(int index) 
{ 
    if (index == 0) 
        return cos(u_light0PenumbraAngle);
    else if (index == 1) 
        return cos(u_light1PenumbraAngle);
    else 
        return cos(u_light2PenumbraAngle);
}
