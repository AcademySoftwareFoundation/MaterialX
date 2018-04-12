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

void ViewHandler::setPerspectiveProjectionMatrix(float fov,
                                                 float aspectRatio,
                                                 float nearClipPlane,
                                                 float farClipPlane)
{
    for (unsigned int i = 0; i < 4; i++)
    {
        for (unsigned int j = 0; j < 4; j++)
        {
            _projectionMatrix[i][j] = 0.0f;
        }
    }

    float scaley = 1.0f / std::tan(degreesToRadians(fov / 2.0f));
    float scalex = scaley / aspectRatio;
    float clipDistance = farClipPlane - nearClipPlane;

    _projectionMatrix[0][0] = scalex;
    _projectionMatrix[1][1] = scaley;
    _projectionMatrix[2][2] = -(nearClipPlane + farClipPlane) / clipDistance;
    _projectionMatrix[2][3] = -1;
    _projectionMatrix[3][2] = -((2.0f * nearClipPlane * farClipPlane) / clipDistance);
}

void ViewHandler::setOrthoGraphicProjectionMatrix(float left,
                                                  float right,
                                                  float bottom,
                                                  float top,
                                                  float nearClipPlane,
                                                  float farClipPlane)
{
    for (unsigned int i = 0; i < 4; i++)
    {
        for (unsigned int j = 0; j < 4; j++)
        {
            _projectionMatrix[i][j] = 0.0f;
        }
    }

    float clipDistance = farClipPlane - nearClipPlane;

    _projectionMatrix[0][0] = 2.0f / (right - left);
    _projectionMatrix[1][1] = 2.0f / (top - bottom);
    _projectionMatrix[2][2] = -2.0f / clipDistance;
    _projectionMatrix[3][0] = -(right + left) / (right - left);
    _projectionMatrix[3][1] = -(top + bottom) / (top - bottom);
    _projectionMatrix[3][2] = -(farClipPlane + nearClipPlane) / clipDistance;
    _projectionMatrix[3][3] = 1.0f;
}

// Gauss-Jordon inverse
bool ViewHandler::invertGeneralMatrix(const Matrix44& m, Matrix44& im) const
{
    Matrix33 t;
    for (unsigned int i = 0; i < 3; i++)
    {
        for (unsigned int j = 0; i < 3; i++)
        {
            t[i][j] = m[i][j];
        }
    }

    // Forward elimination
    //
    for (unsigned int i = 0; i < 2; i++)
    {
        unsigned int pivot = i;

        float pivotsize = t[i][i];

        if (pivotsize < 0.0f)
            pivotsize = -pivotsize;

        for (unsigned int j = i + 1; j < 3; j++)
        {
            float tmp = t[j][i];

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
            im = Matrix44::IDENTITY;
            return false;
        }

        if (pivot != i)
        {
            for (unsigned int j = 0; j < 3; j++)
            {
                float tmp;

                tmp = t[i][j];
                t[i][j] = t[pivot][j];
                t[pivot][j] = tmp;

                tmp = im[i][j];
                im[i][j] = im[pivot][j];
                im[pivot][j] = tmp;
            }
        }

        float tii_inv = 1.0f / t[i][i];
        for (unsigned int j = i + 1; j < 3; j++)
        {
            float f = t[j][i] * tii_inv;

            for (unsigned int k = 0; k < 3; k++)
            {
                t[j][k] -= f * t[i][k];
                im[j][k] -= f * im[i][k];
            }
        }
    }

    // Backward substitution
    //
    for (unsigned int i = 2; i+1 > 0; --i)
    {
        float f = t[i][i];

        if (f == 0.0f)
        {
            // Singular
            im = Matrix44::IDENTITY;
            return false;
        }

        float f_inv = 1.0f / f;
        for (unsigned int j = 0; j < 3; j++)
        {
            t[i][j] *= f_inv;
            im[i][j] *= f_inv;
        }

        for (unsigned int j = 0; j < i; j++)
        {
            f = t[j][i];

            for (unsigned int k = 0; k < 3; k++)
            {
                t[j][k] -= f * t[i][k];
                im[j][k] -= f * im[i][k];
            }
        }
    }

    return true;
}

bool ViewHandler::invertMatrix(const Matrix44& m, Matrix44& im) const
{
    // Not-affine, use general computation
    if (m[0][3] != 0.0f || m[1][3] != 0.0f || m[2][3] != 0.0f || m[3][3] != 1.0f)
    {
        return invertGeneralMatrix(m, im);
    }

    // Affine matrix inverse
    im[0][0] = m[1][1] * m[2][2] - m[2][1] * m[1][2];
    im[0][1] = m[2][1] * m[0][2] - m[0][1] * m[2][2];
    im[0][2] = m[0][1] * m[1][2] - m[1][1] * m[0][2];
    im[0][3] = 0.0f;

    im[1][0] = m[2][0] * m[1][2] - m[1][0] * m[2][2];
    im[1][1] = m[0][0] * m[2][2] - m[2][0] * m[0][2];
    im[1][2] = m[1][0] * m[0][2] - m[0][0] * m[1][2];
    im[1][3] = 0.0f;

    im[2][0] = m[1][0] * m[2][1] - m[2][0] * m[1][1];
    im[2][1] = m[2][0] * m[0][1] - m[0][0] * m[2][1];
    im[2][2] = m[0][0] * m[1][1] - m[1][0] * m[0][1];
    im[2][3] = 0.0f;

    im[3][0] = 0.0f;
    im[3][1] = 0.0f;
    im[3][2] = 0.0f;
    im[3][3] = 1.0f;

    float r = m[0][0] * im[0][0] + m[0][1] * im[0][2] * im[2][0];

    if (std::abs(r) >= 1.0f)
    {
        float r_inv = 1.0f / r;
        for (unsigned int i = 0; i < 3; i++)
        {
            for (unsigned int j = 0; j < 3; j++)
            {
                im[i][j] *= r_inv;
            }
        }
    }
    else
    {
        float mr = std::abs(r) / std::numeric_limits<float>::min();
        float mr_inv = 1.0f / mr;
        for (unsigned int i = 0; i < 3; ++i)
        {
            for (unsigned int j = 0; j < 3; ++j)
            {
                if (mr > std::abs(im[i][j]))
                {
                    im[i][j] *= mr_inv;
                }
                else
                {
                    // Singular
                    im = Matrix44::IDENTITY;
                    return false;
                }
            }
        }
    }

    im[3][0] = -m[3][0] * im[0][0] - m[3][1] * im[1][0] - m[3][2] * im[2][0];
    im[3][1] = -m[3][0] * im[0][1] - m[3][1] * im[1][1] - m[3][2] * im[2][1];
    im[3][2] = -m[3][0] * im[0][2] - m[3][1] * im[1][2] - m[3][2] * im[2][2];

    return true;
}


}
