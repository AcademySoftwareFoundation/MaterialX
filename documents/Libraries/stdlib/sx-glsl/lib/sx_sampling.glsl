//
// Function to compute the sample size relative to a texture coordinate
//
vec2 sx_compute_sample_size_uv(vec2 uv, float filterSize, float filterOffset)
{
   vec2 derivUVx = dFdx(uv) * 0.5f;
   vec2 derivUVy = dFdy(uv) * 0.5f;
   float derivX = abs(derivUVx.x) + abs(derivUVy.x);
   float derivY = abs(derivUVx.y) + abs(derivUVy.y);
   float sampleSizeU = 2.0f * filterSize * derivX + filterOffset;
   if (sampleSizeU < 1.0E-05f)
       sampleSizeU = 1.0E-05f;
   float sampleSizeV = 2.0f * filterSize * derivY + filterOffset;
   if (sampleSizeV < 1.0E-05f)
       sampleSizeV = 1.0E-05f;
   return vec2(sampleSizeU, sampleSizeV);
};

//
// Compute a normal mapped to 0..1 space based on a set of input
// samples using a Sobel filter.
//
vec3 sx_normal_from_samples_sobel(float S[9], float _scale)
{
   float nx = S[0] - S[2] + (2.0*S[3]) - (2.0*S[5]) + S[6] - S[8];
   float ny = S[0] + (2.0*S[1]) + S[2] - S[6] - (2.0*S[7]) - S[8];
   float nz = _scale * sqrt(1.0 - nx*nx - ny*ny);
   vec3 norm = normalize(vec3(nx, ny, nz));
   return (norm + 1.0) * 0.5;
};

//
// Blur using box filter for float samples
//
float sx_blur_box_float(float S[9])
{
    return (S[0] + S[1] + S[2] + S[3] + S[4] + S[5] + S[6] + S[7] + S[8]) / 9.0;
}

//
// Blur using box filter for vec2 samples
//
vec2 sx_blur_box_vec2(vec2 S[9])
{
    return (S[0] + S[1] + S[2] + S[3] + S[4] + S[5] + S[6] + S[7] + S[8]) / 9.0;
}

//
// Blur using box filter for vec3 samples
//
vec3 sx_blur_box_vec3(vec3 S[9])
{
    return (S[0] + S[1] + S[2] + S[3] + S[4] + S[5] + S[6] + S[7] + S[8]) / 9.0;
}

//
// Blur using box filter for vec4 samples
//
vec4 sx_blur_box_vec4(vec4 S[9])
{
    return (S[0] + S[1] + S[2] + S[3] + S[4] + S[5] + S[6] + S[7] + S[8]) / 9.0;
}

//
// Blur using box filter for float samples
//
float sx_blur_gaussian_float(float S[9])
{
    float W[9] = { 0.0625, 0.125, 0.0625, 0.125, 0.25, 0.125, 0.0625, 0.125, 0.0625 };
    return (S[0]*W[0] + S[1]*W[1] + S[2]*W[2] + S[3]*W[3] + S[4]*W[4] + S[5]*W[5] + S[6]*W[6] + S[7]*W[7] + S[8]*W[8]);
}

//
// Blur using gaussian filter for vec2 samples
//
vec2 sx_blur_gaussian_vec2(vec2 S[9])
{
    float W[9] = { 0.0625, 0.125, 0.0625, 0.125, 0.25, 0.125, 0.0625, 0.125, 0.0625 };
    return (S[0]*W[0] + S[1]*W[1] + S[2]*W[2] + S[3]*W[3] + S[4]*W[4] + S[5]*W[5] + S[6]*W[6] + S[7]*W[7] + S[8]*W[8]);
}

//
// Blur using gaussian filter for vec3 samples
//
vec3 sx_blur_gaussian_vec3(vec3 S[9])
{
    float W[9] = { 0.0625, 0.125, 0.0625, 0.125, 0.25, 0.125, 0.0625, 0.125, 0.0625 };
    return (S[0]*W[0] + S[1]*W[1] + S[2]*W[2] + S[3]*W[3] + S[4]*W[4] + S[5]*W[5] + S[6]*W[6] + S[7]*W[7] + S[8]*W[8]);
}

//
// Blur using gaussian filter for vec4 samples
//
vec4 sx_blur_gaussian_vec4(vec4 S[9])
{
    float W[9] = { 0.0625, 0.125, 0.0625, 0.125, 0.25, 0.125, 0.0625, 0.125, 0.0625 };
    return (S[0]*W[0] + S[1]*W[1] + S[2]*W[2] + S[3]*W[3] + S[4]*W[4] + S[5]*W[5] + S[6]*W[6] + S[7]*W[7] + S[8]*W[8]);
}
