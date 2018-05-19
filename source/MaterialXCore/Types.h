//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_TYPES_H
#define MATERIALX_TYPES_H

/// @file
/// Data type classes

#include <MaterialXCore/Library.h>

#include <array>

namespace MaterialX
{

extern const string DEFAULT_TYPE_STRING;
extern const string FILENAME_TYPE_STRING;
extern const string GEOMNAME_TYPE_STRING;
extern const string SURFACE_SHADER_TYPE_STRING;
extern const string VOLUME_SHADER_TYPE_STRING;
extern const string MULTI_OUTPUT_TYPE_STRING;
extern const string NONE_TYPE_STRING;
extern const string VALUE_STRING_TRUE;
extern const string VALUE_STRING_FALSE;
extern const string NAME_PREFIX_SEPARATOR;
extern const string NAME_PATH_SEPARATOR;
extern const string ARRAY_VALID_SEPARATORS;
extern const string ARRAY_PREFERRED_SEPARATOR;

/// The base class for vectors of scalar values
class VectorBase { };

/// A tag class for constructing vectors and matrices without initialization
class Uninit { };

/// The class template for vectors of scalar values.  Inherited by Vector2,
/// Vector3, Vector4, Color2, Color3, and Color4.
///
/// Template parameter V is the vector subclass, S is the scalar element type,
/// and N is the number of scalar elements in the vector.
template <class V, class S, size_t N> class VectorN : public VectorBase
{
  public:
    using Iterator = typename std::array<S, N>::iterator;
    using ConstIterator = typename std::array<S, N>::const_iterator;

  public:
    VectorN() : _arr{} { }
    explicit VectorN(Uninit) { }
    explicit VectorN(S s) { _arr.fill(s); }
    explicit VectorN(const std::array<S, N>& arr) : _arr(arr) { }
    explicit VectorN(const vector<float>& vec) { std::copy_n(vec.begin(), N, _arr.begin()); }

    /// @}
    /// @name Equality Operators
    /// @{

    /// Return true if the given vector is identical to this one.
    bool operator==(const V& rhs) const { return _arr == rhs._arr; }

    /// Return true if the given vector differs from this one.
    bool operator!=(const V& rhs) const { return _arr != rhs._arr; }

    /// @}
    /// @name Indexing Operators
    /// @{

    /// Return the scalar value at the given index.
    S& operator[](size_t i) { return _arr.at(i); }

    /// Return the const scalar value at the given index.
    const S& operator[](size_t i) const { return _arr.at(i); }

    /// @}
    /// @name Component-wise Operators
    /// @{

    /// Component-wise addition of two vectors.
    V operator+(const V& rhs) const
    {
        V res(Uninit{});
        for (size_t i = 0; i < N; i++)
            res[i] = _arr[i] + rhs[i];
        return res;
    }

    /// Component-wise addition of two vectors.
    VectorN& operator+=(const V& rhs)
    {
        *this = *this + rhs;
        return *this;
    }

    /// Component-wise subtraction of two vectors.
    V operator-(const V& rhs) const
    {
        V res(Uninit{});
        for (size_t i = 0; i < N; i++)
            res[i] = _arr[i] - rhs[i];
        return res;
    }

    /// Component-wise subtraction of two vectors.
    VectorN& operator-=(const V& rhs)
    {
        *this = *this - rhs;
        return *this;
    }

    /// Component-wise multiplication of two vectors.
    V operator*(const V& rhs) const
    {
        V res(Uninit{});
        for (size_t i = 0; i < N; i++)
            res[i] = _arr[i] * rhs[i];
        return res;
    }

    /// Component-wise multiplication of two vectors.
    VectorN& operator*=(const V& rhs)
    {
        *this = *this * rhs;
        return *this;
    }

    /// Component-wise division of two vectors.
    V operator/(const V& rhs) const
    {
        V res(Uninit{});
        for (size_t i = 0; i < N; i++)
            res[i] = _arr[i] / rhs[i];
        return res;
    }

    /// Component-wise division of two vectors.
    VectorN& operator/=(const V& rhs)
    {
        *this = *this / rhs;
        return *this;
    }

    /// Component-wise multiplication of a vector by a scalar.
    V operator*(S s) const
    {
        V res(Uninit{});
        for (size_t i = 0; i < N; i++)
            res[i] = _arr[i] * s;
        return res;
    }

    /// Component-wise multiplication of a vector by a scalar.
    V& operator*=(S s)
    {
        *this = *this * s;
        return *this;
    }

    /// Component-wise division of a vector by a scalar.
    V operator/(S s) const
    {
        V res(Uninit{});
        for (size_t i = 0; i < N; i++)
            res[i] = _arr[i] / s;
        return res;
    }

    /// Component-wise division of a vector by a scalar.
    V& operator/=(S s)
    {
        *this = *this / s;
        return *this;
    }

    /// @}
    /// @name Geometric Methods
    /// @{

    /// Return the magnitude of the vector.
    S getMagnitude() const;

    /// Return a normalized vector.
    V getNormalized() const
    {
        return *this / getMagnitude();
    }

    /// @}
    /// @name Iterators
    /// @{

    Iterator begin() { return _arr.begin(); }
    ConstIterator begin() const { return _arr.begin(); }

    Iterator end() { return _arr.end(); }
    ConstIterator end() const { return _arr.end(); }

    /// @}
    /// @name Static Methods
    /// @{

    /// Return the number of scalar elements for the vector.
    static constexpr size_t numElements() { return N; }

    /// @}

  protected:
    std::array<S, N> _arr;
};

/// @class Vector2
/// A vector of two floating-point values
class Vector2 : public VectorN<Vector2, float, 2>
{
  public:
    using VectorN<Vector2, float, 2>::VectorN;
    Vector2() { }
    Vector2(float x, float y) : VectorN(Uninit{})
    {
        _arr = {x, y};
    }
};

/// @class Vector3
/// A vector of three floating-point values
class Vector3 : public VectorN<Vector3, float, 3>
{
  public:
    using VectorN<Vector3, float, 3>::VectorN;
    Vector3() { }
    Vector3(float x, float y, float z) : VectorN(Uninit{})
    {
        _arr = {x, y, z};
    }
};

/// @class Vector4
/// A vector of four floating-point values
class Vector4 : public VectorN<Vector4, float, 4>
{
  public:
    using VectorN<Vector4, float, 4>::VectorN;
    Vector4() { }
    Vector4(float x, float y, float z, float w) : VectorN(Uninit{})
    {
        _arr = {x, y, z, w};
    }
};

/// @class Color2
/// A two-component color value
class Color2 : public Vector2
{
  public:
    using Vector2::Vector2;
};

/// @class Color3
/// A three-component color value
class Color3 : public Vector3
{
  public:
    using Vector3::Vector3;
};

/// @class Color4
/// A four-component color value
class Color4 : public Vector4
{
  public:
    using Vector4::Vector4;
};

/// The base class for square matrices of scalar values
class MatrixBase { };

/// The class template for square matrices of scalar values.  Inherited by
/// Matrix33 and Matrix44.
///
/// The elements of a MatrixN are stored in row-major order, and may be
/// accessed using the syntax <c>matrix[row][column]</c>.
///
/// Template parameter M is the matrix subclass, S is the scalar element type,
/// and N is the number of rows and columns in the matrix.
template <class M, class S, size_t N> class MatrixN : public MatrixBase
{
  public:
    using RowArray = typename std::array<S, N>;
    using Iterator = typename std::array<RowArray, N>::iterator;
    using ConstIterator = typename std::array<RowArray, N>::const_iterator;

  public:
    MatrixN() : _arr{} { }
    explicit MatrixN(Uninit) { }
    explicit MatrixN(S s) { for (RowArray& row : _arr) row.fill(s); }

    /// @}
    /// @name Equality Operators
    /// @{

    /// Return true if the given matrix is identical to this one.
    bool operator==(const M& rhs) const { return _arr == rhs._arr; }

    /// Return true if the given vector differs from this one.
    bool operator!=(const M& rhs) const { return _arr != rhs._arr; }

    /// Return true if the given matrix is equivalent to another
    /// matrix within a given floating point tolerance
    bool isEquivalent(const M& rhs, S tolerance)
    {
        for (size_t i = 0; i < N; i++)
        {
            for (size_t j = 0; j < N; j++)
            {
                if (std::abs(_arr[i][j] - rhs[i][j]) > tolerance)
                {
                    return false;
                }
            }
        }
        return true;
    }

    /// @}
    /// @name Indexing Operators
    /// @{

    /// Return the row array at the given index.
    RowArray& operator[](size_t i) { return _arr.at(i); }

    /// Return the const row array at the given index.
    const RowArray& operator[](size_t i) const { return _arr.at(i); }

    /// @}
    /// @name Component-wise Operators
    /// @{

    /// Component-wise addition of two matrices.
    M operator+(const M& rhs) const
    {
        M res(Uninit{});
        for (size_t i = 0; i < N; i++)
            for (size_t j = 0; j < N; j++)
                res[i][j] = _arr[i][j] + rhs[i][j];
        return res;
    }

    /// Component-wise addition of two matrices.
    MatrixN& operator+=(const M& rhs)
    {
        *this = *this + rhs;
        return *this;
    }

    /// Component-wise subtraction of two matrices.
    M operator-(const M& rhs) const
    {
        M res(Uninit{});
        for (size_t i = 0; i < N; i++)
            for (size_t j = 0; j < N; j++)
                res[i][j] = _arr[i][j] - rhs[i][j];
        return res;
    }

    /// Component-wise subtraction of two matrices.
    MatrixN& operator-=(const M& rhs)
    {
        *this = *this - rhs;
        return *this;
    }

    /// Component-wise multiplication of a matrix and a scalar.
    M operator*(S s) const
    {
        M res(Uninit{});
        for (size_t i = 0; i < N; i++)
            for (size_t j = 0; j < N; j++)
                res[i][j] = _arr[i][j] * s;
        return res;
    }

    /// Component-wise multiplication of a matrix and a scalar.
    MatrixN& operator*=(S s)
    {
        *this = *this * s;
        return *this;
    }

    /// Component-wise division of a matrix by a scalar.
    M operator/(S s) const
    {
        M res(Uninit{});
        for (size_t i = 0; i < N; i++)
            for (size_t j = 0; j < N; j++)
                res[i][j] = _arr[i][j] / s;
        return res;
    }

    /// Component-wise division of a matrix by a scalar.
    MatrixN& operator/=(S s)
    {
        *this = *this / s;
        return *this;
    }

    /// @}
    /// @name Matrix Algebra
    /// @{

    /// Compute the matrix product.
    M operator*(const M& rhs) const
    {
        M res;
        for (size_t i = 0; i < N; i++)
            for (size_t j = 0; j < N; j++)
                for (size_t k = 0; k < N; k++)
                    res[i][j] += _arr[i][k] * rhs[k][j];
        return res;
    }

    /// Compute the matrix product.
    MatrixN& operator*=(const M& rhs)
    {
        *this = *this * rhs;
        return *this;
    }

    /// Divide the first matrix by the second (computed as the product of the
    /// first matrix and the inverse of the second).
    M operator/(const M& rhs) const
    {
        return *this * rhs.getInverse();
    }

    /// Divide the first matrix by the second (computed as the product of the
    /// first matrix and the inverse of the second).
    MatrixN& operator/=(const M& rhs)
    {
        *this *= rhs.getInverse();
        return *this;
    }

    /// Return the transpose of the matrix.
    M getTranspose() const;

    /// Return the determinant of the matrix.
    S getDeterminant() const;

    /// Return the adjugate of the matrix.
    M getAdjugate() const;

    /// Return the inverse of the matrix.
    M getInverse() const
    {
        return getAdjugate() / getDeterminant();
    }

    /// @}
    /// @name Iterators
    /// @{

    Iterator begin() { return _arr.begin(); }
    ConstIterator begin() const { return _arr.begin(); }

    Iterator end() { return _arr.end(); }
    ConstIterator end() const { return _arr.end(); }

    /// @}
    /// @name Static Methods
    /// @{

    /// Return the number of rows in this matrix.
    static constexpr size_t numRows() { return N; }

    /// Return the number of columns in this matrix.
    static constexpr size_t numColumns() { return N; }

    /// @}

  protected:
    std::array<RowArray, N> _arr;
};

/// @class Matrix33
/// A 3x3 matrix of floating-point values
class Matrix33 : public MatrixN<Matrix33, float, 3>
{
  public:
    using MatrixN<Matrix33, float, 3>::MatrixN;
    Matrix33() { }
    Matrix33(float m00, float m01, float m02,
             float m10, float m11, float m12,
             float m20, float m21, float m22) :
        MatrixN(Uninit{})
    {
        _arr = {m00, m01, m02,
                m10, m11, m12,
                m20, m21, m22};
    }

    /// @name 2D Transformations
    /// @{

    /// Create a translation matrix.
    static Matrix33 createTranslation(const Vector2& v);

    /// Create a scale matrix.
    static Matrix33 createScale(const Vector2& v);

    // Create a rotation matrix.
    // @param angle Angle in radians
    static Matrix33 createRotation(float angle);
    
    /// @}

  public:
    static const Matrix33 IDENTITY;
};

/// @class Matrix44
/// A 4x4 matrix of floating-point values
class Matrix44 : public MatrixN<Matrix44, float, 4>
{
  public:
    using MatrixN<Matrix44, float, 4>::MatrixN;
    Matrix44() { }
    Matrix44(float m00, float m01, float m02, float m03,
             float m10, float m11, float m12, float m13,
             float m20, float m21, float m22, float m23,
             float m30, float m31, float m32, float m33) :
        MatrixN(Uninit{})
    {
        _arr = {m00, m01, m02, m03,
                m10, m11, m12, m13,
                m20, m21, m22, m23,
                m30, m31, m32, m33};
    }

    /// @name 3D Transformations
    /// @{

    /// Create a translation matrix.
    static Matrix44 createTranslation(const Vector3& v);

    /// Create a scale matrix.
    static Matrix44 createScale(const Vector3& v);

    /// Create a rotation matrix about the X-axis.
    /// @param angle Angle in radians
    static Matrix44 createRotationX(float angle);

    /// Create a rotation matrix about the Y-axis.
    /// @param angle Angle in radians
    static Matrix44 createRotationY(float angle);

    /// Create a rotation matrix about the Z-axis.
    /// @param angle Angle in radians
    static Matrix44 createRotationZ(float angle);

    /// @}

  public:
    static const Matrix44 IDENTITY;
};

} // namespace MaterialX

#endif
