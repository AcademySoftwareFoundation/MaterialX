#define M_FLOAT_EPS 1e-8

#define mx_sin metal::sin
#define mx_cos metal::cos
#define mx_tan metal::tan
#define mx_asin metal::asin
#define mx_acos metal::acos

float mx_square(float x)
{
    return x*x;
}

vec2 mx_square(vec2 x)
{
    return x*x;
}

vec3 mx_square(vec3 x)
{
    return x*x;
}

float mx_inversesqrt(float x)
{
    return metal::rsqrt(x);
}

template<class T1, class T2>
T1 mx_mod(T1 x, T2 y)
{
    return x - y * floor(x/y);
}

float3x3 mx_inverse(float3x3 m)
{
    float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0];
    float n21 = m[0][1], n22 = m[1][1], n23 = m[2][1];
    float n31 = m[0][2], n32 = m[1][2], n33 = m[2][2];

    float det = metal::determinant(m);
    float idet = 1.0f / det;

    float3x3 ret;

    ret[0][0] = idet * (n22 * n33 - n32 * n23);
    ret[1][0] = idet * (n32 * n13 - n12 * n33);
    ret[2][0] = idet * (n12 * n23 - n22 * n13);
    
    ret[0][1] = idet * (n31 * n23 - n21 * n33);
    ret[1][1] = idet * (n11 * n33 - n31 * n13);
    ret[2][1] = idet * (n21 * n13 - n11 * n23);
    
    ret[0][2] = idet * (n21 * n32 - n31 * n22);
    ret[1][2] = idet * (n31 * n12 - n11 * n32);
    ret[2][2] = idet * (n11 * n22 - n21 * n12);

    return ret;
}

float4x4 mx_inverse(float4x4 m)
{
    float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0], n14 = m[3][0];
    float n21 = m[0][1], n22 = m[1][1], n23 = m[2][1], n24 = m[3][1];
    float n31 = m[0][2], n32 = m[1][2], n33 = m[2][2], n34 = m[3][2];
    float n41 = m[0][3], n42 = m[1][3], n43 = m[2][3], n44 = m[3][3];

    float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
    float t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
    float t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
    float t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

    float det = metal::determinant(m);
    float idet = 1.0f / det;

    float4x4 ret;

    ret[0][0] = t11 * idet;
    ret[0][1] = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * idet;
    ret[0][2] = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * idet;
    ret[0][3] = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * idet;

    ret[1][0] = t12 * idet;
    ret[1][1] = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * idet;
    ret[1][2] = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * idet;
    ret[1][3] = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * idet;

    ret[2][0] = t13 * idet;
    ret[2][1] = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * idet;
    ret[2][2] = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * idet;
    ret[2][3] = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * idet;

    ret[3][0] = t14 * idet;
    ret[3][1] = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * idet;
    ret[3][2] = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * idet;
    ret[3][3] = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * idet;

    return ret;
}

float mx_atan(float y_over_x)
{
    return metal::atan(y_over_x);
}

float mx_atan(float y, float x)
{
    return metal::atan2(y, x);
}

vec2 mx_atan(vec2 y, vec2 x)
{
    return metal::atan2(y, x);
}

vec3 mx_atan(vec3 y, vec3 x)
{
    return metal::atan2(y, x);
}

vec4 mx_atan(vec4 y, vec4 x)
{
    return metal::atan2(y, x);
}

float mx_radians(float degree)
{
    return (degree * M_PI_F / 180.0f);
}
