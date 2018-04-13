//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Types.h>
#include <cmath>

namespace MaterialX
{

const string DEFAULT_TYPE_STRING = "color3";
const string FILENAME_TYPE_STRING = "filename";
const string GEOMNAME_TYPE_STRING = "geomname";
const string SURFACE_SHADER_TYPE_STRING = "surfaceshader";
const string VOLUME_SHADER_TYPE_STRING = "volumeshader";
const string MULTI_OUTPUT_TYPE_STRING = "multioutput";
const string NONE_TYPE_STRING = "none";
const string VALUE_STRING_TRUE = "true";
const string VALUE_STRING_FALSE = "false";
const string NAME_PATH_SEPARATOR = "/";
const string ARRAY_VALID_SEPARATORS = ", ";
const string ARRAY_PREFERRED_SEPARATOR = ", ";

const Matrix33 Matrix33::IDENTITY{1, 0, 0,
                                  0, 1, 0,
                                  0, 0, 1};

const Matrix44 Matrix44::IDENTITY{1, 0, 0, 0,
                                  0, 1, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1};

//
// Matrix33 methods
//

template <> float MatrixN<Matrix33, Vector3, 3>::getDeterminant() const
{
    return _arr[0][0] * (_arr[1][1]*_arr[2][2] - _arr[2][1]*_arr[1][2]) +
           _arr[0][1] * (_arr[1][2]*_arr[2][0] - _arr[2][2]*_arr[1][0]) +
           _arr[0][2] * (_arr[1][0]*_arr[2][1] - _arr[2][0]*_arr[1][1]);
}

template <> Matrix33 MatrixN<Matrix33, Vector3, 3>::getAdjugate() const
{
    return Matrix33(
        _arr[1][1]*_arr[2][2] - _arr[2][1]*_arr[1][2],
        _arr[2][1]*_arr[0][2] - _arr[0][1]*_arr[2][2],
        _arr[0][1]*_arr[1][2] - _arr[1][1]*_arr[0][2],
        _arr[1][2]*_arr[2][0] - _arr[2][2]*_arr[1][0],
        _arr[2][2]*_arr[0][0] - _arr[0][2]*_arr[2][0],
        _arr[0][2]*_arr[1][0] - _arr[1][2]*_arr[0][0],
        _arr[1][0]*_arr[2][1] - _arr[2][0]*_arr[1][1],
        _arr[2][0]*_arr[0][1] - _arr[0][0]*_arr[2][1],
        _arr[0][0]*_arr[1][1] - _arr[1][0]*_arr[0][1]);
}

Matrix33& Matrix33::rotate(float angle)
{
    Matrix33 rotation(Matrix33::IDENTITY);
    float sine = std::sin(angle);
    float cosine = std::cos(angle);

    rotation[0][0] = cosine;
    rotation[0][1] = -sine;
    rotation[1][0] = sine;
    rotation[1][1] = cosine;

    *this = *this * rotation;
    return *this;
}

Matrix33& Matrix33::invertGeneral()
{
    Matrix33 result(*this);
    Matrix33 t(*this);

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
            throw Exception("Cannot invert singular matrix");
        }

        if (pivot != i)
        {
            for (unsigned int j = 0; j < 3; j++)
            {
                float tmp;

                tmp = t[i][j];
                t[i][j] = t[pivot][j];
                t[pivot][j] = tmp;

                tmp = result[i][j];
                result[i][j] = result[pivot][j];
                result[pivot][j] = tmp;
            }
        }

        float tii_inv = 1.0f / t[i][i];
        for (unsigned int j = i + 1; j < 3; j++)
        {
            float f = t[j][i] * tii_inv;

            for (unsigned int k = 0; k < 3; k++)
            {
                t[j][k] -= f * t[i][k];
                result[j][k] -= f * result[i][k];
            }
        }
    }

    // Backward substitution
    //
    for (unsigned int i = 2; i + 1 > 0; --i)
    {
        float f = t[i][i];

        if (f == 0.0f)
        {
            // Singular
            throw Exception("Cannot invert singular matrix");
        }

        float f_inv = 1.0f / f;
        for (unsigned int j = 0; j < 3; j++)
        {
            t[i][j] *= f_inv;
            result[i][j] *= f_inv;
        }

        for (unsigned int j = 0; j < i; j++)
        {
            f = t[j][i];

            for (unsigned int k = 0; k < 3; k++)
            {
                t[j][k] -= f * t[i][k];
                result[j][k] -= f * result[i][k];
            }
        }
    }

    *this = result;
    return *this;
}

Matrix33& Matrix33::invert()
{
    if (_arr[0][2] != 0.0f || _arr[1][2] != 0.0f || _arr[2][2] != 1.0f)
    {
        Matrix33 s(_arr[1][1] * _arr[2][2] - _arr[2][1] * _arr[1][2],
            _arr[2][1] * _arr[0][2] - _arr[0][1] * _arr[2][2],
            _arr[0][1] * _arr[1][2] - _arr[1][1] * _arr[0][2],

            _arr[2][0] * _arr[1][2] - _arr[1][0] * _arr[2][2],
            _arr[0][0] * _arr[2][2] - _arr[2][0] * _arr[0][2],
            _arr[1][0] * _arr[0][2] - _arr[0][0] * _arr[1][2],

            _arr[1][0] * _arr[2][1] - _arr[2][0] * _arr[1][1],
            _arr[2][0] * _arr[0][1] - _arr[0][0] * _arr[2][1],
            _arr[0][0] * _arr[1][1] - _arr[1][0] * _arr[0][1]);

        float r = _arr[0][0] * s[0][0] + _arr[0][1] * s[1][0] + _arr[0][2] * s[2][0];
        float r_inv = 1.0f / r;

        if (std::abs(r) >= 1.0f)
        {
            for (unsigned int i = 0; i < 3; ++i)
            {
                for (unsigned int j = 0; j < 3; ++j)
                {
                    s[i][j] *= r_inv;
                }
            }
        }
        else
        {
            float mr = std::abs(r) / std::numeric_limits<float>::min();
            for (int i = 0; i < 3; ++i)
            {
                for (int j = 0; j < 3; ++j)
                {
                    if (mr > std::abs(s[i][j]))
                    {
                        s[i][j] *= r_inv;
                    }
                    else
                    {
                        // Singular
                        throw Exception("Cannot invert singular matrix");
                    }
                }
            }
        }

        *this = s;
        return *this;
    }

    Matrix33 s(_arr[1][1], -_arr[0][1], 0.0f,
        -_arr[1][0], _arr[0][0], 0.0f,
        0.0f, 0.0f, 1.0f);

    float r = _arr[0][0] * _arr[1][1] - _arr[1][0] * _arr[0][1];
    float r_inv = 1.0f / r;

    if (std::abs(r) >= 1)
    {
        for (unsigned int i = 0; i < 2; ++i)
        {
            for (unsigned int j = 0; j < 2; ++j)
            {
                s[i][j] *= r_inv;
            }
        }
    }
    else
    {
        float mr = std::abs(r) / std::numeric_limits<float>::min();
        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 2; ++j)
            {
                if (mr > std::abs(s[i][j]))
                {
                    s[i][j] *= r_inv;
                }
                else
                {
                    // Singular
                    throw Exception("Cannot invert singular matrix");
                }
            }
        }
    }

    s[2][0] = -_arr[2][0] * s[0][0] - _arr[2][1] * s[1][0];
    s[2][1] = -_arr[2][0] * s[0][1] - _arr[2][1] * s[1][1];
    
    *this = s;
    return *this;
}

//
// Matrix44 methods
//

template <> float MatrixN<Matrix44, Vector4, 4>::getDeterminant() const
{
    return _arr[0][0] * (_arr[1][1]*_arr[2][2]*_arr[3][3] + _arr[3][1]*_arr[1][2]*_arr[2][3] + _arr[2][1]*_arr[3][2]*_arr[1][3] -
                         _arr[1][1]*_arr[3][2]*_arr[2][3] - _arr[2][1]*_arr[1][2]*_arr[3][3] - _arr[3][1]*_arr[2][2]*_arr[1][3]) +
           _arr[0][1] * (_arr[1][2]*_arr[3][3]*_arr[2][0] + _arr[2][2]*_arr[1][3]*_arr[3][0] + _arr[3][2]*_arr[2][3]*_arr[1][0] -
                         _arr[1][2]*_arr[2][3]*_arr[3][0] - _arr[3][2]*_arr[1][3]*_arr[2][0] - _arr[2][2]*_arr[3][3]*_arr[1][0]) +
           _arr[0][2] * (_arr[1][3]*_arr[2][0]*_arr[3][1] + _arr[3][3]*_arr[1][0]*_arr[2][1] + _arr[2][3]*_arr[3][0]*_arr[1][1] -
                         _arr[1][3]*_arr[3][0]*_arr[2][1] - _arr[2][3]*_arr[1][0]*_arr[3][1] - _arr[3][3]*_arr[2][0]*_arr[1][1]) +
           _arr[0][3] * (_arr[1][0]*_arr[3][1]*_arr[2][2] + _arr[2][0]*_arr[1][1]*_arr[3][2] + _arr[3][0]*_arr[2][1]*_arr[1][2] -
                         _arr[1][0]*_arr[2][1]*_arr[3][2] - _arr[3][0]*_arr[1][1]*_arr[2][2] - _arr[2][0]*_arr[3][1]*_arr[1][2]); 
}

template <> Matrix44 MatrixN<Matrix44, Vector4, 4>::getAdjugate() const
{
    return Matrix44(
        _arr[1][1]*_arr[2][2]*_arr[3][3] + _arr[3][1]*_arr[1][2]*_arr[2][3] + _arr[2][1]*_arr[3][2]*_arr[1][3] -
        _arr[1][1]*_arr[3][2]*_arr[2][3] - _arr[2][1]*_arr[1][2]*_arr[3][3] - _arr[3][1]*_arr[2][2]*_arr[1][3],

        _arr[0][1]*_arr[3][2]*_arr[2][3] + _arr[2][1]*_arr[0][2]*_arr[3][3] + _arr[3][1]*_arr[2][2]*_arr[0][3] -
        _arr[3][1]*_arr[0][2]*_arr[2][3] - _arr[2][1]*_arr[3][2]*_arr[0][3] - _arr[0][1]*_arr[2][2]*_arr[3][3],

        _arr[0][1]*_arr[1][2]*_arr[3][3] + _arr[3][1]*_arr[0][2]*_arr[1][3] + _arr[1][1]*_arr[3][2]*_arr[0][3] -
        _arr[0][1]*_arr[3][2]*_arr[1][3] - _arr[1][1]*_arr[0][2]*_arr[3][3] - _arr[3][1]*_arr[1][2]*_arr[0][3],

        _arr[0][1]*_arr[2][2]*_arr[1][3] + _arr[1][1]*_arr[0][2]*_arr[2][3] + _arr[2][1]*_arr[1][2]*_arr[0][3] -
        _arr[0][1]*_arr[1][2]*_arr[2][3] - _arr[2][1]*_arr[0][2]*_arr[1][3] - _arr[1][1]*_arr[2][2]*_arr[0][3],

        _arr[1][2]*_arr[3][3]*_arr[2][0] + _arr[2][2]*_arr[1][3]*_arr[3][0] + _arr[3][2]*_arr[2][3]*_arr[1][0] -
        _arr[1][2]*_arr[2][3]*_arr[3][0] - _arr[3][2]*_arr[1][3]*_arr[2][0] - _arr[2][2]*_arr[3][3]*_arr[1][0],

        _arr[0][2]*_arr[2][3]*_arr[3][0] + _arr[3][2]*_arr[0][3]*_arr[2][0] + _arr[2][2]*_arr[3][3]*_arr[0][0] -
        _arr[0][2]*_arr[3][3]*_arr[2][0] - _arr[2][2]*_arr[0][3]*_arr[3][0] - _arr[3][2]*_arr[2][3]*_arr[0][0],

        _arr[0][2]*_arr[3][3]*_arr[1][0] + _arr[1][2]*_arr[0][3]*_arr[3][0] + _arr[3][2]*_arr[1][3]*_arr[0][0] -
        _arr[0][2]*_arr[1][3]*_arr[3][0] - _arr[3][2]*_arr[0][3]*_arr[1][0] - _arr[1][2]*_arr[3][3]*_arr[0][0],

        _arr[0][2]*_arr[1][3]*_arr[2][0] + _arr[2][2]*_arr[0][3]*_arr[1][0] + _arr[1][2]*_arr[2][3]*_arr[0][0] -
        _arr[0][2]*_arr[2][3]*_arr[1][0] - _arr[1][2]*_arr[0][3]*_arr[2][0] - _arr[2][2]*_arr[1][3]*_arr[0][0],

        _arr[1][3]*_arr[2][0]*_arr[3][1] + _arr[3][3]*_arr[1][0]*_arr[2][1] + _arr[2][3]*_arr[3][0]*_arr[1][1] -
        _arr[1][3]*_arr[3][0]*_arr[2][1] - _arr[2][3]*_arr[1][0]*_arr[3][1] - _arr[3][3]*_arr[2][0]*_arr[1][1],

        _arr[0][3]*_arr[3][0]*_arr[2][1] + _arr[2][3]*_arr[0][0]*_arr[3][1] + _arr[3][3]*_arr[2][0]*_arr[0][1] -
        _arr[0][3]*_arr[2][0]*_arr[3][1] - _arr[3][3]*_arr[0][0]*_arr[2][1] - _arr[2][3]*_arr[3][0]*_arr[0][1],

        _arr[0][3]*_arr[1][0]*_arr[3][1] + _arr[3][3]*_arr[0][0]*_arr[1][1] + _arr[1][3]*_arr[3][0]*_arr[0][1] -
        _arr[0][3]*_arr[3][0]*_arr[1][1] - _arr[1][3]*_arr[0][0]*_arr[3][1] - _arr[3][3]*_arr[1][0]*_arr[0][1],

        _arr[0][3]*_arr[2][0]*_arr[1][1] + _arr[1][3]*_arr[0][0]*_arr[2][1] + _arr[2][3]*_arr[1][0]*_arr[0][1] -
        _arr[0][3]*_arr[1][0]*_arr[2][1] - _arr[2][3]*_arr[0][0]*_arr[1][1] - _arr[1][3]*_arr[2][0]*_arr[0][1],

        _arr[1][0]*_arr[3][1]*_arr[2][2] + _arr[2][0]*_arr[1][1]*_arr[3][2] + _arr[3][0]*_arr[2][1]*_arr[1][2] -
        _arr[1][0]*_arr[2][1]*_arr[3][2] - _arr[3][0]*_arr[1][1]*_arr[2][2] - _arr[2][0]*_arr[3][1]*_arr[1][2],

        _arr[0][0]*_arr[2][1]*_arr[3][2] + _arr[3][0]*_arr[0][1]*_arr[2][2] + _arr[2][0]*_arr[3][1]*_arr[0][2] -
        _arr[0][0]*_arr[3][1]*_arr[2][2] - _arr[2][0]*_arr[0][1]*_arr[3][2] - _arr[3][0]*_arr[2][1]*_arr[0][2],

        _arr[0][0]*_arr[3][1]*_arr[1][2] + _arr[1][0]*_arr[0][1]*_arr[3][2] + _arr[3][0]*_arr[1][1]*_arr[0][2] -
        _arr[0][0]*_arr[1][1]*_arr[3][2] - _arr[3][0]*_arr[0][1]*_arr[1][2] - _arr[1][0]*_arr[3][1]*_arr[0][2],

        _arr[0][0]*_arr[1][1]*_arr[2][2] + _arr[2][0]*_arr[0][1]*_arr[1][2] + _arr[1][0]*_arr[2][1]*_arr[0][2] -
        _arr[0][0]*_arr[2][1]*_arr[1][2] - _arr[1][0]*_arr[0][1]*_arr[2][2] - _arr[2][0]*_arr[1][1]*_arr[0][2]);
}

Matrix44& Matrix44::rotateX(float angle)
{
    Matrix44 rotation(Matrix44::IDENTITY);
    float sine = std::sin(angle);
    float cosine = std::cos(angle);

    rotation[1][1] = cosine;
    rotation[1][2] = -sine;
    rotation[2][1] = sine;
    rotation[2][2] = cosine;

    *this = *this * rotation;
    return *this;
}

Matrix44& Matrix44::rotateY(float angle)
{
    Matrix44 rotation(Matrix44::IDENTITY);
    float sine = std::sin(angle);
    float cosine = std::cos(angle);

    rotation[0][0] = cosine;
    rotation[0][2] = sine;
    rotation[2][0] = -sine;
    rotation[2][2] = cosine;

    *this = *this * rotation;
    return *this;
}


Matrix44& Matrix44::rotateZ(float angle)
{
    Matrix44 rotation(Matrix44::IDENTITY);
    float sine = std::sin(angle);
    float cosine = std::cos(angle);

    rotation[0][0] = cosine;
    rotation[0][1] = -sine;
    rotation[1][0] = sine;
    rotation[1][1] = cosine;

    *this = *this * rotation;
    return *this;
}

Matrix44& Matrix44::invertGeneral()
{
    Matrix44 result(*this);
    Matrix44 t(*this);

    // Forward elimination
    //
    for (unsigned int i = 0; i < 3; i++)
    {
        unsigned int pivot = i;

        float pivotsize = t[i][i];

        if (pivotsize < 0.0f)
            pivotsize = -pivotsize;

        for (unsigned int j = i + 1; j < 4; j++)
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
            throw Exception("Cannot invert singular matrix");
        }

        if (pivot != i)
        {
            for (unsigned int j = 0; j < 4; j++)
            {
                float tmp;

                tmp = t[i][j];
                t[i][j] = t[pivot][j];
                t[pivot][j] = tmp;

                tmp = result[i][j];
                result[i][j] = result[pivot][j];
                result[pivot][j] = tmp;
            }
        }

        float tii_inv = 1.0f / t[i][i];
        for (unsigned int j = i + 1; j < 4; j++)
        {
            float f = t[j][i] * tii_inv;

            for (unsigned int k = 0; k < 3; k++)
            {
                t[j][k] -= f * t[i][k];
                result[j][k] -= f * result[i][k];
            }
        }
    }

    // Backward substitution
    //
    for (unsigned int i = 3; i + 1 > 0; --i)
    {
        float f = t[i][i];

        if (f == 0.0f)
        {
            // Singular
            throw Exception("Cannot invert singular matrix");
        }

        float f_inv = 1.0f / f;
        for (unsigned int j = 0; j < 3; j++)
        {
            t[i][j] *= f_inv;
            result[i][j] *= f_inv;
        }

        for (unsigned int j = 0; j < i; j++)
        {
            f = t[j][i];

            for (unsigned int k = 0; k < 3; k++)
            {
                t[j][k] -= f * t[i][k];
                result[j][k] -= f * result[i][k];
            }
        }
    }

    return *this;
}

Matrix44& Matrix44::invert()
{
    // Not-affine, use general computation
    if (_arr[0][3] != 0.0f || _arr[1][3] != 0.0f || _arr[2][3] != 0.0f || _arr[3][3] != 1.0f)
    {
        return invertGeneral();
    }

    // Affine matrix inverse
    Matrix44 im;

    im[0][0] = _arr[1][1] * _arr[2][2] - _arr[2][1] * _arr[1][2];
    im[0][1] = _arr[2][1] * _arr[0][2] - _arr[0][1] * _arr[2][2];
    im[0][2] = _arr[0][1] * _arr[1][2] - _arr[1][1] * _arr[0][2];
    im[0][3] = 0.0f;

    im[1][0] = _arr[2][0] * _arr[1][2] - _arr[1][0] * _arr[2][2];
    im[1][1] = _arr[0][0] * _arr[2][2] - _arr[2][0] * _arr[0][2];
    im[1][2] = _arr[1][0] * _arr[0][2] - _arr[0][0] * _arr[1][2];
    im[1][3] = 0.0f;

    im[2][0] = _arr[1][0] * _arr[2][1] - _arr[2][0] * _arr[1][1];
    im[2][1] = _arr[2][0] * _arr[0][1] - _arr[0][0] * _arr[2][1];
    im[2][2] = _arr[0][0] * _arr[1][1] - _arr[1][0] * _arr[0][1];
    im[2][3] = 0.0f;

    im[3][0] = 0.0f;
    im[3][1] = 0.0f;
    im[3][2] = 0.0f;
    im[3][3] = 1.0f;

    float r = _arr[0][0] * im[0][0] + _arr[0][1] * im[0][2] * im[2][0];
    float r_inv = 1.0f / r;

    if (std::abs(r) >= 1.0f)
    {
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
        for (unsigned int i = 0; i < 3; ++i)
        {
            for (unsigned int j = 0; j < 3; ++j)
            {
                if (mr > std::abs(im[i][j]))
                {
                    im[i][j] *= r_inv;
                }
                else
                {
                    // Singular
                    throw Exception("Cannot invert singular matrix");
                }
            }
        }
    }

    im[3][0] = -_arr[3][0] * im[0][0] - _arr[3][1] * im[1][0] - _arr[3][2] * im[2][0];
    im[3][1] = -_arr[3][0] * im[0][1] - _arr[3][1] * im[1][1] - _arr[3][2] * im[2][1];
    im[3][2] = -_arr[3][0] * im[0][2] - _arr[3][1] * im[1][2] - _arr[3][2] * im[2][2];

    *this = im;
    return *this;
}


} // namespace MaterialX
