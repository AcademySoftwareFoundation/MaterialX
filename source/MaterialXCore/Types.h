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
#include <istream>
#include <ostream>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
#endif

namespace MaterialX
{

extern const string DEFAULT_TYPE_STRING;
extern const string FILENAME_TYPE_STRING;
extern const string SURFACE_SHADER_TYPE_STRING;
extern const string VOLUME_SHADER_TYPE_STRING;
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
    VectorN() : data{0.0f} { }
    VectorN(float f) { data.fill(f); }
    VectorN(const std::array<float, N>& arr) : data(arr) { }
    VectorN(const vector<float>& vec) { std::copy_n(vec.begin(), N, data.begin()); }

    bool operator==(const VectorN& rhs) const { return data == rhs.data; }
    bool operator!=(const VectorN& rhs) const { return data != rhs.data; }

    float operator[](size_t i) const { return data.at(i); }
    float& operator[](size_t i) { return data.at(i); }

    size_t length() const { return N; }

  public:
    std::array<float, N> data;
};

/// A vector of two floating-point values
class Vector2 : public VectorN<2>
{
  public:
    using VectorN<2>::VectorN;
    Vector2() { }
    Vector2(float x, float y) { data[0] = x; data[1] = y; }
};

/// A vector of three floating-point values
class Vector3 : public VectorN<3>
{
  public:
    using VectorN<3>::VectorN;
    Vector3() { }
    Vector3(float x, float y, float z) { data[0] = x; data[1] = y; data[2] = z; }
};

/// A vector of four floating-point values
class Vector4 : public VectorN<4>
{
  public:
    using VectorN<4>::VectorN;
    Vector4() { }
    Vector4(float x, float y, float z, float w) { data[0] = x; data[1] = y; data[2] = z; data[3] = w; }
};

/// A 3x3 matrix of floating-point values
class Matrix3x3 : public VectorN<9>
{
  public:
    using VectorN<9>::VectorN;
};

/// A 4x4 matrix of floating-point values
class Matrix4x4 : public VectorN<16>
{
  public:
    using VectorN<16>::VectorN;
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

template <std::size_t N> std::istream& operator>>(std::istream& is, VectorN<N>& v)
{
    for (size_t i = 0; i < N; i++)
    {
        is >> v[i];
    }
    return is;
}

template <std::size_t N> std::ostream& operator<<(std::ostream& os, const VectorN<N>& v)
{
    for (size_t i = 0; i < N - (size_t) 1; i++)
    {
        os << v[i] << ARRAY_PREFERRED_SEPARATOR;
    }
    os << v[N - (size_t) 1];
    return os;
}

std::istream& operator>>(std::istream& is, vector<string>& v);
std::ostream& operator<<(std::ostream& os, const vector<string>& v);

} // namespace MaterialX

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif
