void mx_heighttonormal_vector3(float height, float scale, vec2 texcoord, out vec3 result)
{
    // Scale factor for parity with traditional Sobel filtering.
    const float SOBEL_SCALE_FACTOR = 1.0 / 16.0;

    // Compute screen-space gradients of the heightfield and texture coordinates.
    vec2 dHdS = vec2(dFdx(height), dFdy(height)) * scale * SOBEL_SCALE_FACTOR;
    vec2 dUdS = vec2(dFdx(texcoord.x), dFdy(texcoord.x));
    vec2 dVdS = vec2(dFdx(texcoord.y), dFdy(texcoord.y));

    // Construct a screen-space tangent frame.
    vec3 tangent = vec3(dUdS.x, dVdS.x, dHdS.x);
    vec3 bitangent = vec3(dUdS.y, dVdS.y, dHdS.y);
    vec3 n = cross(tangent, bitangent);

    // Handle invalid and mirrored texture coordinates.
    if (dot(n, n) < M_FLOAT_EPS * M_FLOAT_EPS)
    {
        n = vec3(0, 0, 1);
    }
    else if (n.z < 0.0)
    {
        n *= -1.0;
    }

    // Normalize and encode the results.
    result = normalize(n) * 0.5 + 0.5;
}
