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
    VectorN() : data{} { }

    /// An explicit constructor, initializing all scalar elements to the given
    /// value.
    explicit VectorN(S s) { data.fill(s); }

    /// An explicit constructor, initializing the vector from a standard array
    /// the same length.
    explicit VectorN(const std::array<S, N>& arr) : data(arr) { }

    /// An explicit constructor, initializing the vector from a standard vector
    /// of the same length.
    explicit VectorN(const vector<float>& vec) { std::copy_n(vec.begin(), N, data.begin()); }

    /// @}
    /// @name Equality operators
    /// @{

    /// Return true if the given vector is identical to this one.
    bool operator==(const VectorN& rhs) const { return data == rhs.data; }

    /// Return true if the given vector differs from this one.
    bool operator!=(const VectorN& rhs) const { return data != rhs.data; }

    /// @}
    /// @name Indexing operators
    /// @{

    /// Return the scalar value at the given index.
    S operator[](size_t i) const { return data.at(i); }

    /// Return a reference to the scalar value at the given index.
    S& operator[](size_t i) { return data.at(i); }

    /// @}
    /// @name Component-wise operators
    /// @{

    /// Component-wise addition, returning a new vector.
    V operator+(const VectorN& rhs) const
    {
        V res;
        for (size_t i = 0; i < N; i++)
            res[i] = data[i] + rhs[i];
        return res;
    }

    /// Component-wise addition, modifying this vector in place.
    VectorN& operator+=(const VectorN& rhs)
    {
        for (size_t i = 0; i < N; i++)
            data[i] += rhs[i];
        return *this;
    }

    /// Component-wise subtraction, returning a new vector.
    V operator-(const VectorN& rhs) const
    {
        V res;
        for (size_t i = 0; i < N; i++)
            res[i] = data[i] - rhs[i];
        return res;
    }

    /// Component-wise subtraction, modifying this vector in place.
    VectorN& operator-=(const VectorN& rhs)
    {
        for (size_t i = 0; i < N; i++)
            data[i] -= rhs[i];
        return *this;
    }

    /// Component-wise multiplication, returning a new vector.
    V operator*(const VectorN& rhs) const
    {
        V res;
        for (size_t i = 0; i < N; i++)
            res[i] = data[i] * rhs[i];
        return res;
    }

    /// Component-wise multiplication, modifying this vector in place.
    VectorN& operator*=(const VectorN& rhs)
    {
        for (size_t i = 0; i < N; i++)
            data[i] *= rhs[i];
        return *this;
    }

    /// Component-wise division, returning a new vector.
    V operator/(const VectorN& rhs) const
    {
        V res;
        for (size_t i = 0; i < N; i++)
            res[i] = data[i] / rhs[i];
        return res;
    }

    /// Component-wise division, modifying this vector in place.
    VectorN& operator/=(const VectorN& rhs)
    {
        for (size_t i = 0; i < N; i++)
            data[i] /= rhs[i];
        return *this;
    }

    /// @}
    /// @name Iterators
    /// @{

    Iterator begin() { return data.begin(); }
    ConstIterator begin() const { return data.begin(); }

    Iterator end() { return data.end(); }
    ConstIterator end() const { return data.end(); }

    /// @}
    /// @name Utility
    /// @{

    /// Return the internal data for this vector as a standard array.
    std::array<S, N>& getArray() { return data; }

    /// Return the length of this vector.
    static constexpr size_t length() { return N; }

    /// @}

  protected:
    std::array<S, N> data;
};

/// @class Vector2
/// A vector of two floating-point values
class Vector2 : public VectorN<Vector2, float, 2>
{
  public:
    using VectorN<Vector2, float, 2>::VectorN;
    Vector2() { }
    Vector2(float x, float y) { data = {x, y}; }
};

/// @class Vector3
/// A vector of three floating-point values
class Vector3 : public VectorN<Vector3, float, 3>
{
  public:
    using VectorN<Vector3, float, 3>::VectorN;
    Vector3() { }
    Vector3(float x, float y, float z) { data = {x, y, z}; }
};

/// @class Vector4
/// A vector of four floating-point values
class Vector4 : public VectorN<Vector4, float, 4>
{
  public:
    using VectorN<Vector4, float, 4>::VectorN;
    Vector4() { }
    Vector4(float x, float y, float z, float w) { data = {x, y, z, w}; }
};

/// A two-component color value
class Color2 : public Vector2
{
  public:
    using Vector2::Vector2;
};

/// A three-component color value
class Color3 : public Vector3
{
  public:
    using Vector3::Vector3;
};

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
    using VectorType = V;
    using Iterator = typename std::array<V, N>::iterator;
    using ConstIterator = typename std::array<V, N>::const_iterator;

    /// @name Constructors
    /// @{

    /// The default constructor, initializing all scalar elements to their
    /// zero value.
    MatrixN() : data{} { }

    /// An explicit constructor, initializing all scalar elements to the
    /// given value.
    explicit MatrixN(typename V::ScalarType s) { data.fill(V(s)); }

    /// @}
    /// @name Equality operators
    /// @{

    /// Return true if the given matrix is identical to this one.
    bool operator==(const MatrixN& rhs) const { return data == rhs.data; }

    /// Return true if the given vector differs from this one.
    bool operator!=(const MatrixN& rhs) const { return data != rhs.data; }

    /// @}
    /// @name Indexing operators
    /// @{

    /// Return the vector value at the given index.
    V operator[](size_t i) const { return data.at(i); }

    /// Return the vector value at the given index as a reference.
    V& operator[](size_t i) { return data.at(i); }

    /// @}
    /// @name Component-wise operators
    /// @{

    /// Component-wise addition, returning a new matrix.
    M operator+(const MatrixN& rhs) const
    {
        M res;
        for (size_t i = 0; i < N; i++)
            res[i] = data[i] + rhs[i];
        return res;
    }

    /// Component-wise addition, modifying this matrix in place.
    MatrixN& operator+=(const MatrixN& rhs)
    {
        for (size_t i = 0; i < N; i++)
            data[i] += rhs[i];
        return *this;
    }

    /// Component-wise subtraction, returning a new matrix.
    M operator-(const MatrixN& rhs) const
    {
        M res;
        for (size_t i = 0; i < N; i++)
            res[i] = data[i] - rhs[i];
        return res;
    }

    /// Component-wise subtraction, modifying this matrix in place.
    MatrixN& operator-=(const MatrixN& rhs)
    {
        for (size_t i = 0; i < N; i++)
            data[i] -= rhs[i];
        return *this;
    }

    /// Matrix multplication, returning a new matrix.
    M operator*(const MatrixN& rhs) const
    {
        static_assert(numRows() == numColumns(), "Requires a square matrix");
        M res;
        for (size_t i = 0; i < N; i++)
            for (size_t j = 0; j < N; j++)
                for (size_t k = 0; k < N; k++)
                    res[i][j] += data[i][k] * rhs[k][j];
        return res;
    }

    /// Matrix multplication, modifying this matrix in place.
    MatrixN& operator*=(const MatrixN& rhs)
    {
        *this = *this * rhs;
        return *this;
    }

    /// Component-wise division, returning a new matrix.
    /// @todo Add support for matrix division.
    M operator/(const MatrixN& rhs) const
    {
        M res;
        for (size_t i = 0; i < N; i++)
            res[i] = data[i] / rhs[i];
        return res;
    }

    /// Component-wise division, modifying this matrix in place.
    MatrixN& operator/=(const MatrixN& rhs)
    {
        for (size_t i = 0; i < N; i++)
            data[i] /= rhs[i];
        return *this;
    }

    /// @}
    /// @name Iterators
    /// @{

    Iterator begin() { return data.begin(); }
    ConstIterator begin() const { return data.begin(); }

    Iterator end() { return data.end(); }
    ConstIterator end() const { return data.end(); }

    /// @}
    /// @name Utility
    /// @{

    /// Return the given row as a vector.
    V getRow(size_t i) const
    {
        return data[i];
    }

    /// Return the given column as a vector.
    V getColumn(size_t j) const
    {
        V v;
        for (size_t i = 0; i < N; i++)
        {
            v[i] = data[i][j];
        }
        return v;
    }

    /// Return the internal data for this matrix as a standard array.
    std::array<V, N>& getArray() { return data; }

    /// Return the number of rows in this matrix.
    static constexpr size_t numRows() { return N; }

    /// Return the number of columns in this matrix.
    static constexpr size_t numColumns() { return V::length(); }

    /// @}

  protected:
    std::array<V, N> data;
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
        data = {Vector3(m00, m01, m02),
                Vector3(m10, m11, m12),
                Vector3(m20, m21, m22)};
    }

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
        data = {Vector4(m00, m01, m02, m03),
                Vector4(m10, m11, m12, m13),
                Vector4(m20, m21, m22, m23),
                Vector4(m30, m31, m32, m33)};
    }

  public:
    static const Matrix44 IDENTITY;
};

} // namespace MaterialX

#endif
