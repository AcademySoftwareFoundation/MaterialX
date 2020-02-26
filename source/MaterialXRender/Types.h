//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RENDER_TYPES_H
#define MATERIALX_RENDER_TYPES_H

/// @file
/// Data types for rendering functionality

#include <MaterialXCore/Types.h>

namespace MaterialX
{

/// @class Vector3d
/// A vector of three floating-point values (double-precision)
class Vector3d : public VectorN<Vector3d, double, 3>
{
  public:
    using VectorN<Vector3d, double, 3>::VectorN;
    Vector3d() { }
    Vector3d(double x, double y, double z) : VectorN(Uninit{})
    {
        _arr = {x, y, z};
    }
};

/// @class Vector4d
/// A vector of four floating-point values (double-precision)
class Vector4d : public VectorN<Vector4d, double, 4>
{
  public:
    using VectorN<Vector4d, double, 4>::VectorN;
    Vector4d() { }
    Vector4d(double x, double y, double z, double w) : VectorN(Uninit{})
    {
        _arr = {x, y, z, w};
    }
};

/// @class Color3d
/// A three-component color value (double-precision)
class Color3d : public VectorN<Color3d, double, 3>
{
  public:
    using VectorN<Color3d, double, 3>::VectorN;
    Color3d() { }
    Color3d(double r, double g, double b) : VectorN(Uninit{})
    {
        _arr = {r, g, b};
    }
};

/// @class Half
/// A lightweight 16-bit half-precision float class.  Based on the public-domain
/// implementation by Paul Tessier.
class Half
{
  public:
    explicit Half(float value) : _data(toFloat16(value)) { }
    operator float() const { return toFloat32(_data); }

    bool operator==(Half rhs) const { return float(*this) == float(rhs); }
    bool operator!=(Half rhs) const { return float(*this) != float(rhs); }
    bool operator<(Half rhs) const { return float(*this) < float(rhs); }
    bool operator>(Half rhs) const { return float(*this) > float(rhs); }
    bool operator<=(Half rhs) const { return float(*this) <= float(rhs); }
    bool operator>=(Half rhs) const { return float(*this) >= float(rhs); }

    Half operator+(Half rhs) const { return Half(float(*this) + float(rhs)); }
    Half operator-(Half rhs) const { return Half(float(*this) - float(rhs)); }
    Half operator*(Half rhs) const { return Half(float(*this) * float(rhs)); }
    Half operator/(Half rhs) const { return Half(float(*this) / float(rhs)); }

    Half& operator+=(Half rhs) { return operator=(*this + rhs); }
    Half& operator-=(Half rhs) { return operator=(*this - rhs); }
    Half& operator*=(Half rhs) { return operator=(*this * rhs); }
    Half& operator/=(Half rhs) { return operator=(*this / rhs); }

    Half operator-() const { return Half(-float(*this)); }

  private:
    union Bits
    {
        float f;
        int32_t si;
        uint32_t ui;
    };

    static constexpr int const shift = 13;
    static constexpr int const shiftSign = 16;

    static constexpr int32_t const infN = 0x7F800000; // flt32 infinity
    static constexpr int32_t const maxN = 0x477FE000; // max flt16 normal as a flt32
    static constexpr int32_t const minN = 0x38800000; // min flt16 normal as a flt32
    static constexpr int32_t const signN = (int32_t) 0x80000000; // flt32 sign bit

    static constexpr int32_t const infC = infN >> shift;
    static constexpr int32_t const nanN = (infC + 1) << shift; // minimum flt16 nan as a flt32
    static constexpr int32_t const maxC = maxN >> shift;
    static constexpr int32_t const minC = minN >> shift;
    static constexpr int32_t const signC = (int32_t) 0x00008000; // flt16 sign bit

    static constexpr int32_t const mulN = 0x52000000; // (1 << 23) / minN
    static constexpr int32_t const mulC = 0x33800000; // minN / (1 << (23 - shift))

    static constexpr int32_t const subC = 0x003FF; // max flt32 subnormal down shifted
    static constexpr int32_t const norC = 0x00400; // min flt32 normal down shifted

    static constexpr int32_t const maxD = infC - maxC - 1;
    static constexpr int32_t const minD = minC - subC - 1;

    static uint16_t toFloat16(float value)
    {
        Bits v, s;
        v.f = value;
        uint32_t sign = (uint32_t) (v.si & signN);
        v.si ^= sign;
        sign >>= shiftSign; // logical shift
        s.si = mulN;
        s.si = (int32_t) (s.f * v.f); // correct subnormals
        v.si ^= (s.si ^ v.si) & -(minN > v.si);
        v.si ^= (infN ^ v.si) & -((infN > v.si) & (v.si > maxN));
        v.si ^= (nanN ^ v.si) & -((nanN > v.si) & (v.si > infN));
        v.ui >>= shift; // logical shift
        v.si ^= ((v.si - maxD) ^ v.si) & -(v.si > maxC);
        v.si ^= ((v.si - minD) ^ v.si) & -(v.si > subC);
        return (uint16_t) (v.ui | sign);
    }

    static float toFloat32(uint16_t value)
    {
        Bits v;
        v.ui = value;
        int32_t sign = v.si & signC;
        v.si ^= sign;
        sign <<= shiftSign;
        v.si ^= ((v.si + minD) ^ v.si) & -(v.si > subC);
        v.si ^= ((v.si + maxD) ^ v.si) & -(v.si > maxC);
        Bits s;
        s.si = mulC;
        s.f *= float(v.si);
        int32_t mask = (norC > v.si) ? -1 : 1;
        v.si <<= shift;
        v.si ^= (s.si ^ v.si) & mask;
        v.si |= sign;
        return v.f;
    }

  private:
    uint16_t _data;
};

} // namespace MaterialX

#endif
