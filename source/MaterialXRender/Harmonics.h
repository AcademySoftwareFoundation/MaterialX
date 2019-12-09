//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_HARMONICS_H
#define MATERIALX_HARMONICS_H

/// @file
/// Spherical harmonics functionality

#include <MaterialXRender/Image.h>

#include <MaterialXCore/Types.h>

namespace MaterialX
{

/// @class Vector3d
/// A vector of three floating-point values (double precision)
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

/// @class Color3d
/// A three-component color value (double precision)
class Color3d : public Vector3d
{
  public:
    using Vector3d::Vector3d;
};

/// Class template for a vector of spherical harmonic coefficients.
///
/// Template parameter C is the coefficient type (e.g. double, Color3).
/// Template parameter B is the number of spherical harmonic bands.
template <class C, size_t B> class ShCoeffs
{
  public:
    static const size_t NUM_BANDS = B;
    static const size_t NUM_COEFFS = B * B;

  public:
    ShCoeffs() { }
    explicit ShCoeffs(const std::array<C, NUM_COEFFS>& arr) : _arr(arr) { }
    ~ShCoeffs() { }

    /// @name Indexing Operators
    /// @{

    /// Return the coefficient at the given index.
    C& operator[](size_t i) { return _arr.at(i); }

    /// Return the const coefficient at the given index.
    const C& operator[](size_t i) const { return _arr.at(i); }

    /// @}

  protected:
    std::array<C, NUM_COEFFS> _arr;
};

/// Double-precision scalar coefficients for third-order spherical harmonics.
using Sh3ScalarCoeffs = ShCoeffs<double, 3>;

/// Double-precision color coefficients for third-order spherical harmonics.
using Sh3ColorCoeffs = ShCoeffs<Color3d, 3>;

/// Project an environment map to third-order SH, with an optional convolution
///    to convert radiance to irradiance.
/// @param env An environment map in lat-long format.
/// @param irradiance If true, then the returned signal will be convolved
///    by a clamped cosine kernel to generate irradiance.
/// @return The projection of the environment to third-order SH.
Sh3ColorCoeffs projectEnvironment(ConstImagePtr env, bool irradiance);

/// Render the given spherical harmonic signal to an environment map.
/// @param shEnv The color signal of the environment encoded as third-order SH.
/// @param width The width of the output environment map.
/// @param height The height of the output environment map.
/// @return An environment map in the lat-long format.
ImagePtr renderEnvironment(const Sh3ColorCoeffs& shEnv, unsigned int width, unsigned int height);

} // namespace MaterialX

#endif
