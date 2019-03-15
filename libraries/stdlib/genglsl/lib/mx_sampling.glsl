// Restrict to 7x7 kernel size for performance reasons
#define MX_MAX_SAMPLE_COUNT 49
// Size of all weights for all levels (including level 1)
#define MX_WEIGHT_ARRAY_SIZE 84

//
// Function to compute the sample size relative to a texture coordinate
//
vec2 mx_compute_sample_size_uv(vec2 uv, float filterSize, float filterOffset)
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
vec3 mx_normal_from_samples_sobel(float S[9], float _scale)
{
   float nx = S[0] - S[2] + (2.0*S[3]) - (2.0*S[5]) + S[6] - S[8];
   float ny = S[0] + (2.0*S[1]) + S[2] - S[6] - (2.0*S[7]) - S[8];
   float nz = _scale * sqrt(1.0 - nx*nx - ny*ny);
   vec3 norm = normalize(vec3(nx, ny, nz));
   return (norm + 1.0) * 0.5;
};

// Kernel weights for box filter
void mx_get_box_weights(inout float W[MX_MAX_SAMPLE_COUNT], int filterSize)
{
    int sampleCount = filterSize*filterSize;
    float value = 1.0 / float(sampleCount);
    for (int i=0; i<sampleCount; i++)
    {
        W[i] = value;
    }
}

// Kernel weights for Gaussian filter. Sigma is assumed to be 1.
void mx_get_gaussian_weights(inout float W[MX_MAX_SAMPLE_COUNT], int filterSize)
{
    if (filterSize >= 7)
    {
        W[0] = 0.000036;  W[1] = 0.000363;  W[2] = 0.001446;  W[3] = 0.002291;  W[4] = 0.001446;  W[5] = 0.000363;  W[6] = 0.000036;
        W[7] = 0.000363;  W[8] = 0.003676;  W[9] = 0.014662;  W[10] = 0.023226; W[11] = 0.014662; W[12] = 0.003676; W[13] = 0.000363;
        W[14] = 0.001446; W[15] = 0.014662; W[16] = 0.058488; W[17] = 0.092651; W[18] = 0.058488; W[19] = 0.014662; W[20] = 0.001446;
        W[21] = 0.002291; W[22] = 0.023226; W[23] = 0.092651; W[24] = 0.146768; W[25] = 0.092651; W[26] = 0.023226; W[27] = 0.002291;
        W[28] = 0.001446; W[29] = 0.014662; W[30] = 0.058488; W[31] = 0.092651; W[32] = 0.058488; W[33] = 0.014662; W[34] = 0.001446;
        W[35] = 0.000363; W[36] = 0.003676; W[37] = 0.014662; W[38] = 0.023226; W[39] = 0.014662; W[40] = 0.003676; W[41] = 0.000363;
        W[42] = 0.000036; W[43] = 0.000363; W[44] = 0.001446; W[45] = 0.002291; W[46] = 0.001446; W[47] = 0.000363; W[48] = 0.000036;
    }
    else if (filterSize >= 5)
    {
        W[0] = 0.003765;  W[1] = 0.015019;  W[2] = 0.023792;  W[3] = 0.015019;  W[4] = 0.003765;
        W[5] = 0.015019;  W[6] = 0.059912;  W[7] = 0.094907;  W[8] = 0.059912;  W[9] = 0.015019;
        W[10] = 0.023792; W[11] = 0.094907; W[12] = 0.150342; W[13] = 0.094907; W[14] = 0.023792;
        W[15] = 0.015019; W[16] = 0.059912; W[17] = 0.094907; W[18] = 0.059912; W[19] = 0.015019;
        W[20] = 0.003765; W[21] = 0.015019; W[22] = 0.023792; W[23] = 0.015019; W[24] = 0.003765;
    }
    else if (filterSize >= 3)
    {
        W[0] = 0.0625; W[1] = 0.125; W[2] = 0.0625;
        W[3] = 0.125;  W[4] = 0.25;  W[5] = 0.125;
        W[6] = 0.0625; W[7] = 0.125; W[8] = 0.0625;
    }
    else
    {
        W[0] = 1.0;
    }
}

//
// Apply filter for float samples S, using weights W.
// sampleCount should be a square of a odd number in the range { 1, 3, 5, 7 }
//
float mx_convolution_float(float S[MX_MAX_SAMPLE_COUNT], float W[MX_WEIGHT_ARRAY_SIZE], int offset, int sampleCount)
{
    float result = 0.0;
    for (int i = 0;  i < sampleCount; i++)
    {
        result += S[i]*W[i+offset];
    }
    return result;
}

//
// Apply filter for vec2 samples S, using weights W.
// sampleCount should be a square of a odd number in the range { 1, 3, 5, 7 }
//
vec2 mx_convolution_vec2(vec2 S[MX_MAX_SAMPLE_COUNT], float W[MX_WEIGHT_ARRAY_SIZE], int offset, int sampleCount)
{
    vec2 result = vec2(0.0);
    for (int i=0;  i<sampleCount; i++)
    {
        result += S[i]*W[i+offset];
    }
    return result;
}

//
// Apply filter for vec3 samples S, using weights W.
// sampleCount should be a square of a odd number in the range { 1, 3, 5, 7 }
//
vec3 mx_convolution_vec3(vec3 S[MX_MAX_SAMPLE_COUNT], float W[MX_WEIGHT_ARRAY_SIZE], int offset, int sampleCount)
{
    vec3 result = vec3(0.0);
    for (int i=0;  i<sampleCount; i++)
    {
        result += S[i]*W[i+offset];
    }
    return result;
}

//
// Apply filter for vec4 samples S, using weights W.
// sampleCount should be a square of a odd number { 1, 3, 5, 7 }
//
vec4 mx_convolution_vec4(vec4 S[MX_MAX_SAMPLE_COUNT], float W[MX_WEIGHT_ARRAY_SIZE], int offset, int sampleCount)
{
    vec4 result = vec4(0.0);
    for (int i=0;  i<sampleCount; i++)
    {
        result += S[i]*W[i+offset];
    }
    return result;
}
