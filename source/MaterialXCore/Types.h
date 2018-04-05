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

/// The base class for vectors of floating-point values
class VectorBase { };

/// The class template for fixed-length subclasses of VectorBase
template <size_t N> class VectorN : public VectorBase
{
  public:
    VectorN() : data{} { }
    explicit VectorN(float f) { data.fill(f); }
    explicit VectorN(const std::array<float, N>& arr) : data(arr) { }
    explicit VectorN(const vector<float>& vec) { std::copy_n(vec.begin(), N, data.begin()); }

    //
    // Equality operators
    //

    bool operator==(const VectorN& rhs) const { return data == rhs.data; }
    bool operator!=(const VectorN& rhs) const { return data != rhs.data; }

    //
    // Indexing operators
    //

    float operator[](size_t i) const { return data.at(i); }
    float& operator[](size_t i) { return data.at(i); }

    //
    // Component-wise addition
    //

    VectorN operator+(const VectorN& rhs) const
    {
        VectorN res;
        for (size_t i = 0; i < N; i++)
            res[i] = data[i] + rhs[i];
        return res;
    }
    VectorN& operator+=(const VectorN& rhs)
    {
        for (size_t i = 0; i < N; i++)
            data[i] += rhs[i];
        return *this;
    }

    //
    // Component-wise subtraction
    //

    VectorN operator-(const VectorN& rhs) const
    {
        VectorN res;
        for (size_t i = 0; i < N; i++)
            res[i] = data[i] - rhs[i];
        return res;
    }
    VectorN& operator-=(const VectorN& rhs)
    {
        for (size_t i = 0; i < N; i++)
            data[i] -= rhs[i];
        return *this;
    }

    //
    // Component-wise multiplication
    //

    VectorN operator*(const VectorN& rhs) const
    {
        VectorN res;
        for (size_t i = 0; i < N; i++)
            res[i] = data[i] * rhs[i];
        return res;
    }
    VectorN& operator*=(const VectorN& rhs)
    {
        for (size_t i = 0; i < N; i++)
            data[i] *= rhs[i];
        return *this;
    }

    //
    // Component-wise division
    //

    VectorN operator/(const VectorN& rhs) const
    {
        VectorN res;
        for (size_t i = 0; i < N; i++)
            res[i] = data[i] / rhs[i];
        return res;
    }
    VectorN& operator/=(const VectorN& rhs)
    {
        for (size_t i = 0; i < N; i++)
            data[i] /= rhs[i];
        return *this;
    }

    //
    // Iterators
    //

    using iterator = typename std::array<float, N>::iterator;
    using const_iterator = typename std::array<float, N>::const_iterator;

    iterator begin() { return data.begin(); }
    const_iterator begin() const { return data.begin(); }

    iterator end() { return data.end(); }
    const_iterator end() const { return data.end(); }

    //
    // Static methods
    //

    static size_t length() { return N; }

  public:
    std::array<float, N> data;
};

/// A vector of two floating-point values
class Vector2 : public VectorN<2>
{
  public:
    using VectorN<2>::VectorN;
    Vector2() { }
    Vector2(const VectorN<2>& v) { data = v.data; }
    Vector2(float x, float y) { data = {x, y}; }
};

/// A vector of three floating-point values
class Vector3 : public VectorN<3>
{
  public:
    using VectorN<3>::VectorN;
    Vector3() { }
    Vector3(const VectorN<3>& v) { data = v.data; }
    Vector3(float x, float y, float z) { data = {x, y, z}; }
};

/// A vector of four floating-point values
class Vector4 : public VectorN<4>
{
  public:
    using VectorN<4>::VectorN;
    Vector4() { }
    Vector4(const VectorN<4>& v) { data = v.data; }
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

/// The class template for square matrix subclasses of VectorBase
template <size_t N> class MatrixNxN : public VectorN<N*N>
{
  public:
    using VectorN<N*N>::VectorN;

    //
    // Matrix indexing
    //

    float& at(size_t row, size_t col)
    {
        return VectorN<N*N>::data[row * N + col];
    }
    const float& at(size_t row, size_t col) const
    {
        return VectorN<N*N>::data[row * N + col];
    }

    //
    // Matrix multiplication
    //

    MatrixNxN operator*(const MatrixNxN& rhs) const
    {
        MatrixNxN res;
        for (size_t i = 0; i < N; i++)
            for (size_t j = 0; j < N; j++)
                for (size_t k = 0; k < N; k++)
                    res.at(i, j) += at(i, k) * rhs.at(k, j);
        return res;
    }
    MatrixNxN& operator*=(const MatrixNxN& rhs)
    {
        *this = *this * rhs;
        return *this;
    }
};

/// A 3x3 matrix of floating-point values
class Matrix3x3 : public MatrixNxN<3>
{
  public:
    using MatrixNxN<3>::MatrixNxN;
    Matrix3x3() { }
    Matrix3x3(const VectorN<9>& m) { data = m.data; }
    Matrix3x3(float m00, float m01, float m02,
              float m10, float m11, float m12,
              float m20, float m21, float m22)
    {
        data = {m00, m01, m02,
                m10, m11, m12,
                m20, m21, m22};
    }

  public:
    static Matrix3x3 IDENTITY;
};

/// A 4x4 matrix of floating-point values
class Matrix4x4 : public MatrixNxN<4>
{
  public:
    using MatrixNxN<4>::MatrixNxN;
    Matrix4x4() { }
    Matrix4x4(const VectorN<16>& m) { data = m.data; }
    Matrix4x4(float m00, float m01, float m02, float m03,
              float m10, float m11, float m12, float m13,
              float m20, float m21, float m22, float m23,
              float m30, float m31, float m32, float m33)
    {
        data = {m00, m01, m02, m03,
                m10, m11, m12, m13,
                m20, m21, m22, m23,
                m30, m31, m32, m33};
    }

  public:
    static Matrix4x4 IDENTITY;
};

} // namespace MaterialX

#endif
