void mx_heighttonormal_vector3(float height, float scale, vec2 texcoord, out vec3 result)
{
    // Compute the gradient of the heightfield signal with respect
    // to the input texture coordinates.
    vec2 dHdS = vec2(-dFdx(height), dFdy(height)) * scale;
    vec2 dTdS = (abs(dFdx(texcoord)) + abs(dFdy(texcoord))) * 16.0;
    vec2 dHdT = dHdS / dTdS;

    // Convert the gradient to a normal and encode for storage.
    vec3 n = normalize(vec3(dHdT.x, dHdT.y, 1.0));
    result = n * 0.5 + 0.5;
}
