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
    float sine = std::sin(angle);
    float cosine = std::cos(angle);

    Matrix33 rotation{
        cosine, -sine, 0.0f,
        sine, cosine, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    *this = *this * rotation;

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
    float sine = std::sin(angle);
    float cosine = std::cos(angle);
    
    Matrix44 orig(*this);
    _arr[0][1] = orig[0][1] * cosine + orig[0][2] * -sine;
    _arr[0][2] = orig[0][1] * sine + orig[0][2] * cosine;
    _arr[1][1] = orig[1][1] * cosine + orig[1][2] * -sine;
    _arr[1][2] = orig[1][1] * sine + orig[1][2] * cosine;
    _arr[2][1] = orig[2][1] * cosine + orig[2][2] * -sine;
    _arr[2][2] = orig[2][1] * sine + orig[2][2] * cosine;
    _arr[3][1] = orig[3][1] * cosine + orig[3][2] * -sine;
    _arr[3][2] = orig[3][2] * sine + orig[3][2] * cosine;

    return *this;
}

Matrix44& Matrix44::rotateY(float angle)
{
    float sine = std::sin(angle);
    float cosine = std::cos(angle);

    Matrix44 orig(*this);
    _arr[0][0] = orig[0][0] * cosine + orig[0][2] * sine;
    _arr[0][2] = orig[0][0] * -sine + orig[0][2] * cosine;
    _arr[1][0] = orig[1][0] * cosine + orig[1][2] * sine;
    _arr[1][2] = orig[1][0] * -sine + orig[1][2] * cosine;
    _arr[2][0] = orig[2][0] * cosine + orig[2][2] * sine;
    _arr[2][2] = orig[2][0] * -sine + orig[2][2] * cosine;
    _arr[3][0] = orig[3][0] * cosine + orig[3][2] * sine;
    _arr[3][2] = orig[3][0] * -sine + orig[3][2] * cosine;

    return *this;
}

Matrix44& Matrix44::rotateZ(float angle)
{
    float sine = std::sin(angle);
    float cosine = std::cos(angle);

    Matrix44 orig(*this);
    _arr[0][0] = orig[0][0] * cosine + orig[0][1] * sine;
    _arr[0][1] = orig[0][0] * -sine + orig[0][1] * cosine;
    _arr[1][0] = orig[1][0] * cosine + orig[1][1] * sine;
    _arr[1][1] = orig[1][0] * -sine + orig[1][1] * cosine;
    _arr[2][0] = orig[2][0] * cosine + orig[2][1] * sine;
    _arr[2][1] = orig[2][0] * -sine + orig[2][1] * cosine;
    _arr[3][0] = orig[3][0] * cosine + orig[3][1] * sine;
    _arr[3][1] = orig[3][0] * -sine + orig[3][1] * cosine;

    return *this;
}

} // namespace MaterialX
