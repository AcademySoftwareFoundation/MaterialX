//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_TYPES_H
#define MATERIALX_TYPES_H

/// @file
/// Data type classes

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Util.h>

#include <array>
#include <cmath>

namespace MaterialX
{

extern const string DEFAULT_TYPE_STRING;
extern const string FILENAME_TYPE_STRING;
extern const string GEOMNAME_TYPE_STRING;
extern const string STRING_TYPE_STRING;
extern const string SURFACE_SHADER_TYPE_STRING;
extern const string DISPLACEMENT_SHADER_TYPE_STRING;
extern const string VOLUME_SHADER_TYPE_STRING;
extern const string LIGHT_SHADER_TYPE_STRING;
extern const string MATERIAL_TYPE_STRING;
extern const string SURFACE_MATERIAL_NODE_STRING;
extern const string VOLUME_MATERIAL_NODE_STRING;
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
/// Vector3, Vector4, Color3, and Color4.
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
    explicit VectorN(const vector<S>& vec) { std::copy(vec.begin(), vec.end(), _arr.begin()); }
    explicit VectorN(const S* begin, const S* end) { std::copy(begin, end, _arr.begin()); }

    /// @name Comparison Operators
    /// @{

    /// Return true if the given vector is identical to this one.
    bool operator==(const V& rhs) const { return _arr == rhs._arr; }

    /// Return true if the given vector differs from this one.
    bool operator!=(const V& rhs) const { return _arr != rhs._arr; }

    /// Compare two vectors lexicographically.
    bool operator<(const V& rhs) const
    {
        return _arr < rhs._arr;
    }

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
        for (size_t i = 0; i < N; i++)
            _arr[i] += rhs[i];
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
        for (size_t i = 0; i < N; i++)
            _arr[i] -= rhs[i];
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
        for (size_t i = 0; i < N; i++)
            _arr[i] *= rhs[i];
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
        for (size_t i = 0; i < N; i++)
            _arr[i] /= rhs[i];
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
    VectorN& operator*=(S s)
    {
        for (size_t i = 0; i < N; i++)
            _arr[i] *= s;
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
    VectorN& operator/=(S s)
    {
        for (size_t i = 0; i < N; i++)
            _arr[i] /= s;
        return *this;
    }

    /// Unary negation of a vector.
    V operator-() const
    {
        V res(Uninit{});
        for (size_t i = 0; i < N; i++)
            res[i] = -_arr[i];
        return res;
    }

    /// @}
    /// @name Geometric Methods
    /// @{

    /// Return the magnitude of the vector.
    S getMagnitude() const
    {
        S res{};
        for (size_t i = 0; i < N; i++)
            res += _arr[i] * _arr[i];
        return std::sqrt(res);
    }

    /// Return a normalized vector.
    V getNormalized() const
    {
        return *this / getMagnitude();
    }

    /// Return the dot product of two vectors.
    S dot(const V& rhs) const
    {
        S res{};
        for (size_t i = 0; i < N; i++)
            res += _arr[i] * rhs[i];
        return res;
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

    /// Return a pointer to the underlying data array.
    S* data() { return _arr.data(); }

    /// Return a const pointer to the underlying data array.
    const S* data() const { return _arr.data(); }

    /// Function object for hashing vectors.
    class Hash
    {
      public:
        size_t operator()(const V& v) const noexcept
        {
            size_t h = 0;
            for (size_t i = 0; i < N; i++)
                hashCombine(h, v[i]);
            return h;
        }
    };

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

    /// Return the cross product of two vectors.
    float cross(const Vector2& rhs) const
    {
        return _arr[0] * rhs[1] - _arr[1] * rhs[0];
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

    /// Return the cross product of two vectors.
    Vector3 cross(const Vector3& rhs) const
    {
        return Vector3(_arr[1] * rhs[2] - _arr[2] * rhs[1],
                       _arr[2] * rhs[0] - _arr[0] * rhs[2],
                       _arr[0] * rhs[1] - _arr[1] * rhs[0]);
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

/// @class Quaternion
/// A quaternion vector
class Quaternion : public VectorN<Vector4, float, 4>
{
  public:
    using VectorN<Vector4, float, 4>::VectorN;
    Quaternion() { }
    Quaternion(float x, float y, float z, float w) : VectorN(Uninit{})
    {
        _arr = {x, y, z, w};
    }

    Quaternion operator*(const Quaternion& q) const
    {
        return 
        { 
            _arr[0] * q._arr[3] + _arr[3] * q._arr[0] + _arr[1] * q._arr[2] - _arr[2] * q._arr[1], 
            _arr[1] * q._arr[3] + _arr[3] * q._arr[1] + _arr[2] * q._arr[0] - _arr[0] * q._arr[2],
            _arr[2] * q._arr[3] + _arr[3] * q._arr[2] + _arr[0] * q._arr[1] - _arr[1] * q._arr[0], 
            _arr[3] * q._arr[3] - _arr[0] * q._arr[0] - _arr[1] * q._arr[1] - _arr[2] * q._arr[2] 
        };
    }

    Quaternion getNormalized() const
    {
        float l = 1.f / getMagnitude() * (_arr[3] < 0 ? -1.f : 1.f); // after normalization, real part will be non-negative
        return { _arr[0] * l, _arr[1] * l, _arr[2] * l, _arr[3] * l };
    }

    static Quaternion createFromAxisAngle(const Vector3& v, float a)
    {
        float s = std::sin(a * 0.5f);
        return Quaternion(v[0] * s, v[1] * s, v[2] * s, std::cos(a * 0.5f));
    }

  public:
    static const Quaternion IDENTITY;
};

/// @class Color3
/// A three-component color value
class Color3 : public VectorN<Color3, float, 3>
{
  public:
    using VectorN<Color3, float, 3>::VectorN;
    Color3() { }
    Color3(float r, float g, float b) : VectorN(Uninit{})
    {
        _arr = {r, g, b};
    }
};

/// @class Color4
/// A four-component color value
class Color4 : public VectorN<Color4, float, 4>
{
  public:
    using VectorN<Color4, float, 4>::VectorN;
    Color4() { }
    Color4(float r, float g, float b, float a) : VectorN(Uninit{})
    {
        _arr = {r, g, b, a};
    }
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
    explicit MatrixN(S s) { std::fill_n(&_arr[0][0], N * N, s); }
    explicit MatrixN(const S* begin, const S* end) { std::copy(begin, end, &_arr[0][0]); }

    /// @name Comparison Operators
    /// @{

    /// Return true if the given matrix is identical to this one.
    bool operator==(const M& rhs) const { return _arr == rhs._arr; }

    /// Return true if the given matrix differs from this one.
    bool operator!=(const M& rhs) const { return _arr != rhs._arr; }

    /// Return true if the given matrix is equivalent to this one
    /// within a given floating-point tolerance.
    bool isEquivalent(const M& rhs, S tolerance) const
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
    /// @name Utility
    /// @{

    /// Return a pointer to the underlying data array.
    S* data() { return _arr.front().data(); }

    /// Return a const pointer to the underlying data array.
    const S* data() const { return _arr.front().data(); }

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
/// A 3x3 matrix of floating-point values.
///
/// Vector transformation methods follow the row-vector convention,
/// with matrix-vector multiplication computed as v' = vM.
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

    /// @name Vector Transformations
    /// @{

    /// Return the product of this matrix and a 3D vector.
    Vector3 multiply(const Vector3& v) const;

    /// Transform the given 2D point.
    Vector2 transformPoint(const Vector2& v) const;

    /// Transform the given 2D direction vector.
    Vector2 transformVector(const Vector2& v) const;

    /// Transform the given 3D normal vector.
    Vector3 transformNormal(const Vector3& v) const;

    /// Create a translation matrix.
    static Matrix33 createTranslation(const Vector2& v);

    /// Create a scale matrix.
    static Matrix33 createScale(const Vector2& v);

    /// Create a rotation matrix.
    /// @param angle Angle in radians
    static Matrix33 createRotation(float angle);

    /// @}

  public:
    static const Matrix33 IDENTITY;
};

/// @class Matrix44
/// A 4x4 matrix of floating-point values.
///
/// Vector transformation methods follow the row-vector convention,
/// with matrix-vector multiplication computed as v' = vM.
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

    /// @name Vector Transformations
    /// @{

    /// Return the product of this matrix and a 4D vector.
    Vector4 multiply(const Vector4& v) const;

    /// Transform the given 3D point.
    Vector3 transformPoint(const Vector3& v) const;

    /// Transform the given 3D direction vector.
    Vector3 transformVector(const Vector3& v) const;

    /// Transform the given 3D normal vector.
    Vector3 transformNormal(const Vector3& v) const;

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

    /// Create a rotation matrix using a quaternion whose imaginary component is in the
    /// the supplied vectors xyz, and whose real component is in the fourth component, w.
    /// @param quaternion
    static Matrix44 createRotation(const Quaternion& quaternion);

    /// @}

  public:
    static const Matrix44 IDENTITY;
};

} // namespace MaterialX

#endif
