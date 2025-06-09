void mx_heighttonormal_vector3(float height, float scale, vec2 texcoord, out vec3 result)
{
    // Compute the gradient of the heightfield with respect to the screen.
    vec2 dHdS = vec2(dFdx(height), dFdy(height)) * scale;

    // Compute the gradient of the texture coordinates with respect to the screen.
    vec2 dUdS = vec2(dFdx(texcoord.x), dFdy(texcoord.x));
    vec2 dVdS = vec2(dFdx(texcoord.y), dFdy(texcoord.y));

    // Use the chain rule to compute the gradient of the heightfield with
    // respect to the texture coordinates.
    vec2 dHdT;
    float det = dUdS.x * dVdS.y - dUdS.y * dVdS.x;
    if (abs(det) > M_FLOAT_EPS)
    {
        mat2 invJacobian = mat2(-dVdS.y, dUdS.y, dVdS.x, -dUdS.x) / det;
        dHdT = invJacobian * dHdS;
    }
    else
    {
        dHdT = vec2(0.0);
    }

    // Scale the results for parity with traditional Sobel filtering.
    // https://nrsyed.com/2018/02/18/edge-detection-in-images-how-to-derive-the-sobel-operator/
    const float SOBEL_SCALE_FACTOR = 1.0 / 16.0;
    dHdT *= SOBEL_SCALE_FACTOR;

    // Convert the gradient to a normal and encode for storage.
    vec3 n = normalize(vec3(dHdT.x, dHdT.y, 1.0));
    result = n * 0.5 + 0.5;
}
