void mx_heighttonormal_vector3(float height, float scale, vec2 texcoord, out vec3 result)
{
    // Compute the gradient of the heightfield signal with respect
    // to the input texture coordinates.
    vec2 dHdS = vec2(-dFdx(height), dFdy(height)) * scale;
    vec2 dTdSx = dFdx(texcoord);
    vec2 dTdSy = dFdy(texcoord);
    vec2 dTdS = vec2(abs(dTdSx.x) + abs(dTdSy.x),
                     abs(dTdSx.y) + abs(dTdSy.y));
    vec2 dHdT = dHdS / dTdS;

    // Apply a scale factor to match discrete heightfield sampling.
    float DISCRETE_SCALE_FACTOR = 0.04;
    dHdT *= DISCRETE_SCALE_FACTOR;

    // Convert the gradient to a normal and encode for storage.
    vec3 n = normalize(vec3(dHdT.x, dHdT.y, 1.0));
    result = n * 0.5 + 0.5;
}
