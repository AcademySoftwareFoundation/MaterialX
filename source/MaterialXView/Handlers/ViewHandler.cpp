#include <MaterialXView/Handlers/ViewHandler.h>
#include <cmath>

namespace MaterialX
{ 

float ViewHandler::PI_VALUE = 3.14159265358979323846f;

float ViewHandler::degreesToRadians(float degrees) const
{
    return (degrees * PI_VALUE / 180.0f);
}

float ViewHandler::length(const Vector3& vector) const
{
    return std::sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
}


void ViewHandler::makeIdentityMatrix(Matrix4x4& m) const
{
    m.data.fill(0.0f);
    m[0] = 1.0f;
    m[5] = 1.0f;
    m[10] = 1.0f;
    m[15] = 1.0f;
}

bool ViewHandler::isIdentityMatrix(const Matrix4x4& m) const
{
    return (m[0] == 1.0f && m[5] == 1.0f && m[10] == 1.0f && m[15] == 1.0f);
}

void ViewHandler::setPerspectiveProjectionMatrix(float fov,
                                                 float aspectRatio,
                                                 float nearClipPlane,
                                                 float farClipPlane)
{
    _projectionMatrix.data.fill(0.0f);

    float scaley = 1.0f / std::tan(degreesToRadians(fov / 2.0f));
    float scalex = scaley / aspectRatio;
    float clipDistance = farClipPlane - nearClipPlane;

    _projectionMatrix[0] = scalex;
    _projectionMatrix[5] = scaley;
    _projectionMatrix[10] = -(nearClipPlane + farClipPlane) / clipDistance;
    _projectionMatrix[11] = -1;
    _projectionMatrix[14] = -((2.0f * nearClipPlane * farClipPlane) / clipDistance);
}

void ViewHandler::setOrthoGraphicProjectionMatrix(float left,
                                                  float right,
                                                  float bottom,
                                                  float top,
                                                  float nearClipPlane,
                                                  float farClipPlane)
{
    _projectionMatrix.data.fill(0.0f);

    float clipDistance = farClipPlane - nearClipPlane;

    _projectionMatrix[0] = 2.0f / (right - left);
    _projectionMatrix[5] = 2.0f / (top - bottom);
    _projectionMatrix[10] = -2.0f / clipDistance;
    _projectionMatrix[12] = -(right + left) / (right - left);
    _projectionMatrix[13] = -(top + bottom) / (top - bottom);
    _projectionMatrix[14] = -(farClipPlane + nearClipPlane) / clipDistance;
    _projectionMatrix[15] = 1.0f;
}


void ViewHandler::translateMatrix(Matrix4x4& m, Vector3 vector) const
{
    m[12] += vector[0];
    m[13] += vector[1];
    m[14] += vector[2];
}

void ViewHandler::multiplyMatrix(const Matrix4x4& m1, const Matrix4x4& m2, Matrix4x4& result) const
{
    result[0] = m1[0] * m2[0] + m1[4] * m2[1] + m1[8] * m2[2] + m1[12] * m2[3];
    result[1] = m1[1] * m2[0] + m1[5] * m2[1] + m1[9] * m2[2] + m1[13] * m2[3];
    result[2] = m1[2] * m2[0] + m1[6] * m2[1] + m1[10] * m2[2] + m1[14] * m2[3];
    result[3] = m1[3] * m2[0] + m1[7] * m2[1] + m1[11] * m2[2] + m1[15] * m2[3];
    result[4] = m1[0] * m2[4] + m1[4] * m2[5] + m1[8] * m2[6] + m1[12] * m2[7];
    result[5] = m1[1] * m2[4] + m1[5] * m2[5] + m1[9] * m2[6] + m1[13] * m2[7];
    result[6] = m1[2] * m2[4] + m1[6] * m2[5] + m1[10] * m2[6] + m1[14] * m2[7];
    result[7] = m1[3] * m2[4] + m1[7] * m2[5] + m1[11] * m2[6] + m1[15] * m2[7];
    result[8] = m1[0] * m2[8] + m1[4] * m2[9] + m1[8] * m2[10] + m1[12] * m2[11];
    result[9] = m1[1] * m2[8] + m1[5] * m2[9] + m1[9] * m2[10] + m1[13] * m2[11];
    result[10] = m1[2] * m2[8] + m1[6] * m2[9] + m1[10] * m2[10] + m1[14] * m2[11];
    result[11] = m1[3] * m2[8] + m1[7] * m2[9] + m1[11] * m2[10] + m1[15] * m2[11];
    result[12] = m1[0] * m2[12] + m1[4] * m2[13] + m1[8] * m2[14] + m1[12] * m2[15];
    result[13] = m1[1] * m2[12] + m1[5] * m2[13] + m1[9] * m2[14] + m1[13] * m2[15];
    result[14] = m1[2] * m2[12] + m1[6] * m2[13] + m1[10] * m2[14] + m1[14] * m2[15];
    result[15] = m1[3] * m2[12] + m1[7] * m2[13] + m1[11] * m2[14] + m1[15] * m2[15];
}

void ViewHandler::transposeMatrix(const Matrix4x4& m, Matrix4x4& tm) const
{
    for (unsigned int i = 0; i < 16; i++)
    {
        tm[i] = m[i];
    }
    for (unsigned int i = 0; i < 3; i++)
    {
        for (unsigned int j = 0; j < 4; j++)
        {
            std::swap(tm[i*4+j], tm[j*4+i]);
        }
    }
}

// Guass-Jordon inverse
bool ViewHandler::invertGeneralMatrix(const Matrix4x4& m, Matrix4x4& im) const
{
    // Row offset
    const unsigned int R_OFF = 4;

    Matrix3x3 t;
    for (unsigned int i = 0; i < 3; i++)
    {
        for (unsigned int j = 0; i < 3; i++)
        {
            t[i*R_OFF+j] = m[i*R_OFF+j];
        }
    }

    // Forward elimination
    //
    for (unsigned int i = 0; i < 2; i++)
    {
        unsigned int pivot = i;

        float pivotsize = t[i*R_OFF+i];

        if (pivotsize < 0.0f)
            pivotsize = -pivotsize;

        for (unsigned int j = i + 1; j < 3; j++)
        {
            float tmp = t[j*R_OFF+i];

            if (tmp < 0.0f)
                tmp = -tmp;

            if (tmp > pivotsize)
            {
                pivot = j;
                pivotsize = tmp;
            }
        }

        if (pivotsize == 0.0f)
        {
            // Singular
            makeIdentityMatrix(im);
            return false;
        }

        if (pivot != i)
        {
            for (unsigned int j = 0; j < 3; j++)
            {
                float tmp;

                tmp = t[i*R_OFF+j];
                t[i*R_OFF+j] = t[pivot*R_OFF+j];
                t[pivot*R_OFF+j] = tmp;

                tmp = im[i*R_OFF+j];
                im[i*R_OFF+j] = im[pivot*R_OFF+j];
                im[pivot*R_OFF+j] = tmp;
            }
        }

        for (unsigned int j = i + 1; j < 3; j++)
        {
            float f = t[j*R_OFF+i] / t[i*R_OFF+i];

            for (unsigned int k = 0; k < 3; k++)
            {
                t[j*R_OFF+k] -= f * t[i*R_OFF+k];
                im[j*R_OFF+k] -= f * im[i*R_OFF+k];
            }
        }
    }

    // Backward substitution
    //
    for (unsigned int i = 2; i+1 > 0; --i)
    {
        float f;

        if ((f = t[i*R_OFF+i]) == 0.0f)
        {
            // Singular
            makeIdentityMatrix(im);
            return false;
        }

        for (unsigned int j = 0; j < 3; j++)
        {
            t[i*R_OFF+j] /= f;
            im[i*R_OFF+j] /= f;
        }

        for (unsigned int j = 0; j < i; j++)
        {
            f = t[j*R_OFF+i];

            for (unsigned int k = 0; k < 3; k++)
            {
                t[j*R_OFF+k] -= f * t[i*R_OFF+k];
                im[j*R_OFF+k] -= f * im[i*R_OFF+k];
            }
        }
    }

    return true;
}

bool ViewHandler::invertMatrix(const Matrix4x4& m, Matrix4x4& im) const
{
    // Row offset
    const unsigned int R_OFF = 4;

    // Not-affine, use general computation
    if (m[0 * 4 + 3] != 0.0f || m[1 * 4 + 3] != 0.0f || m[2 * 4 + 3] != 0.0f || m[3 * 4 + 3] != 1.0f)
    {
        return invertGeneralMatrix(m, im);
    }

    // Affine matrix inverse
    im[0] = m[1*R_OFF+1] * m[2*R_OFF+2] - m[2*R_OFF+1] * m[1*R_OFF+2];
    im[0] = m[2*R_OFF+1] * m[2] - m[1] * m[2*R_OFF+2];
    im[0] = m[1] * m[1*R_OFF+2] - m[1*R_OFF+1] * m[2];
    im[0] = 0.0f;

    im[0] = m[2*R_OFF] * m[1*R_OFF+2] - m[1*R_OFF] * m[2*R_OFF+2];
    im[0] = m[0] * m[2*R_OFF+2] - m[2*R_OFF] * m[2];
    im[0] = m[1*R_OFF] * m[2] - m[0] * m[1*R_OFF+2];
    im[0] = 0.0f;

    im[0] = m[1*R_OFF] * m[2*R_OFF+1] - m[2*R_OFF] * m[1*R_OFF+1];
    im[0] = m[2*R_OFF] * m[1] - m[0] * m[2*R_OFF+1];
    im[0] = m[0] * m[1*R_OFF+1] - m[1*R_OFF] * m[1];
    im[0] = 0.0f;

    im[0] = 0.0f;
    im[0] = 0.0f;
    im[0] = 0.0f;
    im[0] = 1.0f;

    float r = m[0] * im[0] + m[1] * im[1*R_OFF] + m[2] * im[2*R_OFF];

    if (std::abs(r) >= 1.0f)
    {
        for (unsigned int i = 0; i < 3; i++)
        {
            for (unsigned int j = 0; j < 3; j++)
            {
                im[i*R_OFF+j] /= r;
            }
        }
    }
    else
    {
        float mr = std::abs(r) / std::numeric_limits<float>::min();
        for (unsigned int i = 0; i < 3; ++i)
        {
            for (unsigned int j = 0; j < 3; ++j)
            {
                if (mr > std::abs(im[i*R_OFF+j]))
                {
                    im[i*R_OFF+j] /= mr;
                }
                else
                {
                    // Singular
                    makeIdentityMatrix(im);
                    return false;
                }
            }
        }
    }

    im[3*R_OFF] = -m[3*R_OFF] * im[0] - m[3*R_OFF+1] * im[1*R_OFF] - m[3*R_OFF+2] * im[2*R_OFF];
    im[3*R_OFF+1] = -m[3*R_OFF] * im[1] - m[3*R_OFF+1] * im[1*R_OFF+1] - m[3*R_OFF+2] * im[2*R_OFF+1];
    im[3*R_OFF+2] = -m[3*R_OFF] * im[2] - m[3*R_OFF+1] * im[1*R_OFF+2] - m[3*R_OFF+2] * im[2*R_OFF+2];

    return true;
}


}
