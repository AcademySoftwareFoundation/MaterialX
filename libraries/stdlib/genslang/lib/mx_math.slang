#define M_FLOAT_EPS 1e-8

#define mx_inversesqrt rsqrt
#define mx_sin sin
#define mx_cos cos
#define mx_tan tan
#define mx_asin asin
#define mx_acos acos
#define mx_radians radians
#define mx_float_bits_to_int asint

/// The GLSL we are piggybacking on has all its matrices transposed compared to Slang and the MaterialX spec.
/// (The matrices are defined like mat3(1, 2, 3, 4, 5, 6, 7, 8, 9) where the spec says it should be row-major order, but GLSL creates it as col-major)
/// So when GLSL code says "mul(M, v)" it means "v * transpose(M)", and since in Slang the matrices are stored
/// in row-major order, we need to reverse the order of multiplication to get the same result.
float2 mx_matrix_mul(float2 v, float2x2 m) { return mul(m, v); }
float3 mx_matrix_mul(float3 v, float3x3 m) { return mul(m, v); }
float4 mx_matrix_mul(float4 v, float4x4 m) { return mul(m, v); }
float2 mx_matrix_mul(float2x2 m, float2 v) { return mul(v, m); }
float3 mx_matrix_mul(float3x3 m, float3 v) { return mul(v, m); }
float4 mx_matrix_mul(float4x4 m, float4 v) { return mul(v, m); }
float2x2 mx_matrix_mul(float2x2 m1, float2x2 m2) { return mul(m2, m1); }
float3x3 mx_matrix_mul(float3x3 m1, float3x3 m2) { return mul(m2, m1); }
float4x4 mx_matrix_mul(float4x4 m1, float4x4 m2) { return mul(m2, m1); }

float mx_square(float x)
{
    return x*x;
}

float2 mx_square(float2 x)
{
    return x*x;
}

float3 mx_square(float3 x)
{
    return x*x;
}

float3 mx_srgb_encode(float3 color)
{
    bool3 isAbove = (color > float3(0.0031308));
    float3 linSeg = color * 12.92;
    float3 powSeg = 1.055 * pow(max(color, float3(0.0)), float3(1.0 / 2.4)) - 0.055;
    return select(isAbove, powSeg, linSeg);
}

/// Library assumes GLSL style mod (result has the sign of y), so cannot use fmod
float mx_mod(float a, float b) { return (a - b * floor(a / b)); }
float2 mx_mod(float2 a, float2 b) { return (a - b * floor(a / b)); }
float3 mx_mod(float3 a, float3 b) { return (a - b * floor(a / b)); }
float4 mx_mod(float4 a, float4 b) { return (a - b * floor(a / b)); }
float2 mx_mod(float2 a, float b) { return (a - b * floor(a / b)); }
float3 mx_mod(float3 a, float b) { return (a - b * floor(a / b)); }
float4 mx_mod(float4 a, float b) { return (a - b * floor(a / b)); }

/// The float3x3 and float4x4 inverse are taken from the Slang's SGL library
/// https://github.com/shader-slang/sgl/
/// Specifically the matrix_math.h
/// https://github.com/shader-slang/sgl/blob/main/src/sgl/math/matrix_math.h
float3x3 mx_inverse(float3x3 m)
{
    float one_over_det = 1.f / determinant(m);

    float3x3 result;
    result[0][0] = +(m[1][1] * m[2][2] - m[1][2] * m[2][1]) * one_over_det;
    result[0][1] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]) * one_over_det;
    result[0][2] = +(m[0][1] * m[1][2] - m[0][2] * m[1][1]) * one_over_det;
    result[1][0] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) * one_over_det;
    result[1][1] = +(m[0][0] * m[2][2] - m[0][2] * m[2][0]) * one_over_det;
    result[1][2] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * one_over_det;
    result[2][0] = +(m[1][0] * m[2][1] - m[1][1] * m[2][0]) * one_over_det;
    result[2][1] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * one_over_det;
    result[2][2] = +(m[0][0] * m[1][1] - m[0][1] * m[1][0]) * one_over_det;
    return result;
}

float4x4 mx_inverse(float4x4 m)
{
    float c00 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
    float c02 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
    float c03 = m[2][1] * m[3][2] - m[2][2] * m[3][1];

    float c04 = m[1][2] * m[3][3] - m[1][3] * m[3][2];
    float c06 = m[1][1] * m[3][3] - m[1][3] * m[3][1];
    float c07 = m[1][1] * m[3][2] - m[1][2] * m[3][1];

    float c08 = m[1][2] * m[2][3] - m[1][3] * m[2][2];
    float c10 = m[1][1] * m[2][3] - m[1][3] * m[2][1];
    float c11 = m[1][1] * m[2][2] - m[1][2] * m[2][1];

    float c12 = m[0][2] * m[3][3] - m[0][3] * m[3][2];
    float c14 = m[0][1] * m[3][3] - m[0][3] * m[3][1];
    float c15 = m[0][1] * m[3][2] - m[0][2] * m[3][1];

    float c16 = m[0][2] * m[2][3] - m[0][3] * m[2][2];
    float c18 = m[0][1] * m[2][3] - m[0][3] * m[2][1];
    float c19 = m[0][1] * m[2][2] - m[0][2] * m[2][1];

    float c20 = m[0][2] * m[1][3] - m[0][3] * m[1][2];
    float c22 = m[0][1] * m[1][3] - m[0][3] * m[1][1];
    float c23 = m[0][1] * m[1][2] - m[0][2] * m[1][1];

    float4 fac0 = float4(c00, c00, c02, c03);
    float4 fac1 = float4(c04, c04, c06, c07);
    float4 fac2 = float4(c08, c08, c10, c11);
    float4 fac3 = float4(c12, c12, c14, c15);
    float4 fac4 = float4(c16, c16, c18, c19);
    float4 fac5 = float4(c20, c20, c22, c23);

    float4 vec0 = float4(m[0][1], m[0][0], m[0][0], m[0][0]);
    float4 vec1 = float4(m[1][1], m[1][0], m[1][0], m[1][0]);
    float4 vec2 = float4(m[2][1], m[2][0], m[2][0], m[2][0]);
    float4 vec3 = float4(m[3][1], m[3][0], m[3][0], m[3][0]);

    float4 inv0 = float4(vec1 * fac0 - vec2 * fac1 + vec3 * fac2);
    float4 inv1 = float4(vec0 * fac0 - vec2 * fac3 + vec3 * fac4);
    float4 inv2 = float4(vec0 * fac1 - vec1 * fac3 + vec3 * fac5);
    float4 inv3 = float4(vec0 * fac2 - vec1 * fac4 + vec2 * fac5);

    float4 sign_a = float4(+1, -1, +1, -1);
    float4 sign_b = float4(-1, +1, -1, +1);
    // make matrix from columns
    float4x4 inverse = transpose(float4x4(inv0 * sign_a, inv1 * sign_b, inv2 * sign_a, inv3 * sign_b));

    float4 row0 = float4(inverse[0][0], inverse[0][1], inverse[0][2], inverse[0][3]);
    float4 col0 = float4(inverse[0][0], inverse[1][0], inverse[2][0], inverse[3][0]);

    float4 dot0 = float4(col0 * row0);
    float dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

    float one_over_det = 1.f / dot1;

    return inverse * one_over_det;
}

float mx_atan(float y_over_x)
{
    return atan(y_over_x);
}

float mx_atan(float y, float x)
{
    return atan2(y, x);
}

float2 mx_atan(float2 y, float2 x)
{
    return atan2(y, x);
}

float3 mx_atan(float3 y, float3 x)
{
    return atan2(y, x);
}

float4 mx_atan(float4 y, float4 x)
{
    return atan2(y, x);
}
