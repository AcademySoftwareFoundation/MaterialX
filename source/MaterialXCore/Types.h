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
#include <cmath>

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
extern const string NAME_PATH_SEPARATOR;
extern const string ARRAY_VALID_SEPARATORS;
extern const string ARRAY_PREFERRED_SEPARATOR;

/// The base class for vectors of scalar values
class VectorBase { };

/// The class template for vectors of scalar values.
///
/// Template parameter V is the vector subclass (CRTP), S is the scalar element
/// type, and N is the number of scalar elements in the vector.
template <class V, class S, size_t N> class VectorN : public VectorBase
{
  public:
    using ScalarType = S;
    using Iterator = typename std::array<S, N>::iterator;
    using ConstIterator = typename std::array<S, N>::const_iterator;

    /// @name Constructors
    /// @{

    /// The default constructor, initializing all scalar elements to their
    /// zero value.
    VectorN() : _arr{} { }

    /// An explicit constructor, initializing all scalar elements to the given
    /// value.
    explicit VectorN(S s) { _arr.fill(s); }

    /// An explicit constructor, initializing the vector from a standard array
    /// the same length.
    explicit VectorN(const std::array<S, N>& arr) : _arr(arr) { }

    /// An explicit constructor, initializing the vector from a standard vector
    /// of the same length.
    explicit VectorN(const vector<float>& vec) { std::copy_n(vec.begin(), N, _arr.begin()); }

    /// @}
    /// @name Equality operators
    /// @{

    /// Return true if the given vector is identical to this one.
    bool operator==(const VectorN& rhs) const { return _arr == rhs._arr; }

    /// Return true if the given vector differs from this one.
    bool operator!=(const VectorN& rhs) const { return _arr != rhs._arr; }

    /// @}
    /// @name Indexing operators
    /// @{

    /// Return the scalar value at the given index.
    S operator[](size_t i) const { return _arr.at(i); }

    /// Return a reference to the scalar value at the given index.
    S& operator[](size_t i) { return _arr.at(i); }

    /// @}
    /// @name Component-wise operators
    /// @{

    /// Component-wise addition of two vectors.
    V operator+(const VectorN& rhs) const
    {
        V res;
        for (size_t i = 0; i < N; i++)
            res[i] = _arr[i] + rhs[i];
        return res;
    }

    /// Component-wise addition of two vectors.
    VectorN& operator+=(const VectorN& rhs)
    {
        for (size_t i = 0; i < N; i++)
            _arr[i] += rhs[i];
        return *this;
    }

    /// Component-wise subtraction of two vectors.
    V operator-(const VectorN& rhs) const
    {
        V res;
        for (size_t i = 0; i < N; i++)
            res[i] = _arr[i] - rhs[i];
        return res;
    }

    /// Component-wise subtraction of two vectors.
    VectorN& operator-=(const VectorN& rhs)
    {
        for (size_t i = 0; i < N; i++)
            _arr[i] -= rhs[i];
        return *this;
    }

    /// Component-wise multiplication of two vectors.
    V operator*(const VectorN& rhs) const
    {
        V res;
        for (size_t i = 0; i < N; i++)
            res[i] = _arr[i] * rhs[i];
        return res;
    }

    /// Component-wise multiplication of two vectors.
    VectorN& operator*=(const VectorN& rhs)
    {
        for (size_t i = 0; i < N; i++)
            _arr[i] *= rhs[i];
        return *this;
    }

    /// Component-wise division of two vectors.
    V operator/(const VectorN& rhs) const
    {
        V res;
        for (size_t i = 0; i < N; i++)
            res[i] = _arr[i] / rhs[i];
        return res;
    }

    /// Component-wise division of two vectors.
    VectorN& operator/=(const VectorN& rhs)
    {
        for (size_t i = 0; i < N; i++)
            _arr[i] /= rhs[i];
        return *this;
    }

    /// Component-wise multiplication of a vector by a scalar.
    V operator*(ScalarType s) const
    {
        V res;
        for (size_t i = 0; i < N; i++)
            res[i] = _arr[i] * s;
        return res;
    }

    /// Component-wise multiplication of a vector by a scalar.
    V& operator*=(ScalarType s)
    {
        for (size_t i = 0; i < N; i++)
            _arr[i] *= s;
        return *this;
    }

    /// Component-wise division of a vector by a scalar.
    V operator/(ScalarType s) const
    {
        V res;
        for (size_t i = 0; i < N; i++)
            res[i] = _arr[i] / s;
        return res;
    }

    /// Component-wise division of a vector by a scalar.
    V& operator/=(ScalarType s)
    {
        for (size_t i = 0; i < N; i++)
            _arr[i] /= s;
        return *this;
    }

    /// @}
    /// @name Geometric operators
    /// @{

    /// Magnitude of a vector
    S magnitude() const
    {
        S result = (S)0;
        for (size_t i = 0; i < N; i++)
        {
            result += _arr[i] * _arr[i];
        }
        result = std::sqrt(result);
        return result;
    }

    /// Normalization
    void normalize()
    {
        S mag1 = (S)1 / magnitude();
        for (size_t i = 0; i < N; i++)
        {
            _arr[i] *= mag1;
        }
    }

    /// @}
    /// @name Iterators
    /// @{

    Iterator begin() { return _arr.begin(); }
    ConstIterator begin() const { return _arr.begin(); }

    Iterator end() { return _arr.end(); }
    ConstIterator end() const { return _arr.end(); }

    /// @}
    /// @name Utility
    /// @{

    /// Return the internal data for this vector as a standard array.
    std::array<S, N>& getArray() { return _arr; }

    /// @}
    /// @name Static Methods
    /// @{

    /// Return the length of this vector.
    static constexpr size_t length() { return N; }

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
    Vector2(float x, float y) { _arr = {x, y}; }
};

/// @class Vector3
/// A vector of three floating-point values
class Vector3 : public VectorN<Vector3, float, 3>
{
  public:
    using VectorN<Vector3, float, 3>::VectorN;
    Vector3() { }
    Vector3(float x, float y, float z) { _arr = {x, y, z}; }
};

/// @class Vector4
/// A vector of four floating-point values
class Vector4 : public VectorN<Vector4, float, 4>
{
  public:
    using VectorN<Vector4, float, 4>::VectorN;
    Vector4() { }
    Vector4(float x, float y, float z, float w) { _arr = {x, y, z, w}; }
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

/// The base class for matrices of scalar values
class MatrixBase { };

/// The class template for matrices of scalar values
///
/// Template parameter M is the matrix subclass (CRTP), V is the row vector
/// type, and N is the number of row vectors in the matrix.
template <class M, class V, size_t N> class MatrixN : public MatrixBase
{
  public:
    using ScalarType = typename V::ScalarType;
    using Iterator = typename std::array<V, N>::iterator;
    using ConstIterator = typename std::array<V, N>::const_iterator;

    /// @name Constructors
    /// @{

    /// The default constructor, initializing all scalar elements to their
    /// zero value.
    MatrixN() : _arr{} { }

    /// An explicit constructor, initializing all scalar elements to the
    /// given value.
    explicit MatrixN(ScalarType s) { _arr.fill(V(s)); }

    /// @}
    /// @name Equality operators
    /// @{

    /// Return true if the given matrix is identical to this one.
    bool operator==(const MatrixN& rhs) const { return _arr == rhs._arr; }

    /// Return true if the given vector differs from this one.
    bool operator!=(const MatrixN& rhs) const { return _arr != rhs._arr; }

    /// Return true if the given matrix is equivalent to another
    /// matrix within a given floating point tolerance
    bool equivalent(const MatrixN& rhs, float tolerance)
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
    /// @name Indexing operators
    /// @{

    /// Return the vector value at the given index.
    V operator[](size_t i) const { return _arr.at(i); }

    /// Return the vector value at the given index as a reference.
    V& operator[](size_t i) { return _arr.at(i); }

    /// @}
    /// @name Component-wise operators
    /// @{

    /// Component-wise addition of two matrices.
    M operator+(const MatrixN& rhs) const
    {
        M res;
        for (size_t i = 0; i < N; i++)
            res[i] = _arr[i] + rhs[i];
        return res;
    }

    /// Component-wise addition of two matrices.
    MatrixN& operator+=(const MatrixN& rhs)
    {
        for (size_t i = 0; i < N; i++)
            _arr[i] += rhs[i];
        return *this;
    }

    /// Component-wise subtraction of two matrices.
    M operator-(const MatrixN& rhs) const
    {
        M res;
        for (size_t i = 0; i < N; i++)
            res[i] = _arr[i] - rhs[i];
        return res;
    }

    /// Component-wise subtraction of two matrices.
    MatrixN& operator-=(const MatrixN& rhs)
    {
        for (size_t i = 0; i < N; i++)
            _arr[i] -= rhs[i];
        return *this;
    }

    /// Component-wise multiplication of a matrix and a scalar.
    M operator*(ScalarType s) const
    {
        M res;
        for (size_t i = 0; i < N; i++)
            res[i] = _arr[i] * s;
        return res;
    }

    /// Component-wise multiplication of a matrix and a scalar.
    MatrixN& operator*=(ScalarType s)
    {
        for (size_t i = 0; i < N; i++)
            _arr[i] *= s;
        return *this;
    }

    /// Component-wise division of a matrix by a scalar.
    M operator/(ScalarType s) const
    {
        M res;
        for (size_t i = 0; i < N; i++)
            res[i] = _arr[i] / s;
        return res;
    }

    /// Component-wise division of a matrix by a scalar.
    MatrixN& operator/=(ScalarType s)
    {
        for (size_t i = 0; i < N; i++)
            _arr[i] /= s;
        return *this;
    }

    /// @}
    /// @name Matrix Algebra
    /// @{

    /// Multiply the first matrix by the second.
    M operator*(const MatrixN& rhs) const
    {
        M res;
        for (size_t i = 0; i < N; i++)
            for (size_t j = 0; j < N; j++)
                for (size_t k = 0; k < N; k++)
                    res[i][j] += _arr[i][k] * rhs[k][j];
        return res;
    }

    /// Multiply the first matrix by the second.
    MatrixN& operator*=(const MatrixN& rhs)
    {
        *this = *this * rhs;
        return *this;
    }

    /// Divide the first matrix by the second (computed as the product of the
    /// first matrix and the inverse of the second).
    M operator/(const MatrixN& rhs) const
    {
        return *this * rhs.getInverse();
    }

    /// Divide the first matrix by the second (computed as the product of the
    /// first matrix and the inverse of the second).
    MatrixN& operator/=(const MatrixN& rhs)
    {
        *this *= rhs.getInverse();
        return *this;
    }

    /// Return the determinant of the matrix.
    float getDeterminant() const;

    /// Return the adjugate of the matrix.
    M getAdjugate() const;

    /// Return the inverse of the matrix.
    M getInverse() const
    {
        return getAdjugate() / getDeterminant();
    }

    /// @}
    /// @name 3D Transformations
    /// @{

    /// Set matrix to identity
    void setIdentity()
    {
        for (size_t i = 0; i < N; i++)
        {
            for (size_t j = 0; j < N; j++)
            {
                _arr[i][j] = (i == j) ? (ScalarType)1 : (ScalarType)0;
            }
        }
    }

    /// Set matrix to be a translation. 
    void setTranslation(const V& v)
    {
        setIdentity();
        for (size_t j = 0; j < N - 1; j++)
        {
            _arr[N-1][j] = v[j];
        }
    }

    /// Set matrix to be a scale
    void setScale(const V& v)
    {
        setIdentity();
        for (size_t i = 0; i < N - 1; i++)
        {
            _arr[i][i] = v[i];
        }
    }

    /// Transpose matrix
    MatrixN& transpose()
    {
        for (size_t i = 0; i < N; i++)
        {
            for (size_t j = 0; j < i; j++)
            {
                std::swap(_arr[i][j], _arr[j][i]);
            }
        }
        return *this;
    }

    /// @}
    /// @name Iterators
    /// @{

    Iterator begin() { return _arr.begin(); }
    ConstIterator begin() const { return _arr.begin(); }

    Iterator end() { return _arr.end(); }
    ConstIterator end() const { return _arr.end(); }

    /// @}
    /// @name Utility
    /// @{

    /// Return the given row as a vector.
    V getRow(size_t i) const
    {
        return _arr[i];
    }

    /// Return the given column as a vector.
    V getColumn(size_t j) const
    {
        V v;
        for (size_t i = 0; i < N; i++)
        {
            v[i] = _arr[i][j];
        }
        return v;
    }

    /// Return the internal data for this matrix as a standard array.
    std::array<V, N>& getArray() { return _arr; }

    /// @}
    /// @name Static Methods
    /// @{

    /// Return the number of rows in this matrix.
    static constexpr size_t numRows() { return N; }

    /// Return the number of columns in this matrix.
    static constexpr size_t numColumns() { return V::length(); }

    /// @}

  protected:
    std::array<V, N> _arr;
};

/// @class Matrix33
/// A 3x3 matrix of floating-point values
class Matrix33 : public MatrixN<Matrix33, Vector3, 3>
{
  public:
    using MatrixN<Matrix33, Vector3, 3>::MatrixN;
    Matrix33() { }
    Matrix33(float m00, float m01, float m02,
             float m10, float m11, float m12,
             float m20, float m21, float m22)
    {
        _arr = {Vector3(m00, m01, m02),
                Vector3(m10, m11, m12),
                Vector3(m20, m21, m22)};
    }

    // Set matrix to a given rotation
    void setRotation(float angle);

  public:
    static const Matrix33 IDENTITY;
};

/// @class Matrix44
/// A 4x4 matrix of floating-point values
class Matrix44 : public MatrixN<Matrix44, Vector4, 4>
{
  public:
    using MatrixN<Matrix44, Vector4, 4>::MatrixN;
    Matrix44() { }
    Matrix44(float m00, float m01, float m02, float m03,
             float m10, float m11, float m12, float m13,
             float m20, float m21, float m22, float m23,
             float m30, float m31, float m32, float m33)
    {
        _arr = {Vector4(m00, m01, m02, m03),
                Vector4(m10, m11, m12, m13),
                Vector4(m20, m21, m22, m23),
                Vector4(m30, m31, m32, m33)};
    }

    /// Set matrix to be a given rotation in X
    void setRotationX(float angle);

    /// Set matrix to be a given rotation in Y
    void setRotationY(float angle);

    /// Set matrix to be a given rotation in Z
    void setRotationZ(float angle);

  public:
    static const Matrix44 IDENTITY;
};

/// Component-wise multiplication of a scalar and a vector.
template <class V, class S, size_t N> V operator*(typename V::ScalarType s, const VectorN<V, S, N>& v)
{
    return v * s;
}

/// Component-wise multiplication of a scalar and a matrix.
template <class M, class V, size_t N> M operator*(typename M::ScalarType s, const MatrixN<M, V, N>& m)
{
    return m * s;
}

} // namespace MaterialX

#endif
