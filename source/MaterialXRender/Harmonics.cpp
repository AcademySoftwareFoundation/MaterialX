//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/Harmonics.h>

#include <cmath>

namespace MaterialX
{

namespace {

const double PI = std::acos(-1.0);

const double BASIS_CONSTANT_0 = std::sqrt( 1.0 / ( 4.0 * PI));
const double BASIS_CONSTANT_1 = std::sqrt( 3.0 / ( 4.0 * PI));
const double BASIS_CONSTANT_2 = std::sqrt(15.0 / ( 4.0 * PI));
const double BASIS_CONSTANT_3 = std::sqrt( 5.0 / (16.0 * PI));
const double BASIS_CONSTANT_4 = std::sqrt(15.0 / (16.0 * PI));

const double COSINE_CONSTANT_0 = 1.0;
const double COSINE_CONSTANT_1 = 2.0 / 3.0;
const double COSINE_CONSTANT_2 = 1.0 / 4.0;

double imageXToPhi(unsigned int x, unsigned int width)
{
    // Directions are measured from the center of the pixel, so add 0.5
    // to convert from pixel indices to pixel coordinates.
    return 2.0 * PI * (x + 0.5) / width;
}

double imageYToTheta(unsigned int y, unsigned int height)
{
    return PI * (y + 0.5) / height;
}

Vector3d sphericalToCartesian(double theta, double phi)
{
    double r = std::sin(theta);
    return Vector3d(r * std::cos(phi), r * std::sin(phi), std::cos(theta));
}

double texelSolidAngle(unsigned int y, unsigned int width, unsigned int height)
{
    // Return the solid angle of a texel within a lat-long environment map.
    //
    // Reference:
    //   https://en.wikipedia.org/wiki/Solid_angle#Latitude-longitude_rectangle

    double dTheta = std::cos(y * PI / height) - std::cos((y + 1) * PI / height);
    double dPhi = 2.0 * PI / width;
    return dTheta * dPhi;
}

ShScalarCoeffs evalDirection(const Vector3d& dir)
{
    // Evaluate the spherical harmonic basis functions for the given direction,
    // returning the first three bands of coefficients.
    //
    // References:
    //   https://cseweb.ucsd.edu/~ravir/papers/envmap/envmap.pdf
    //   http://orlandoaguilar.github.io/sh/spherical/harmonics/irradiance/map/2017/02/12/SphericalHarmonics.html

    const double& x = dir[0];
    const double& y = dir[1];
    const double& z = dir[2];

    return ShScalarCoeffs(
    {
        BASIS_CONSTANT_0,
        BASIS_CONSTANT_1 * y,
        BASIS_CONSTANT_1 * z,
        BASIS_CONSTANT_1 * x,
        BASIS_CONSTANT_2 * x * y,
        BASIS_CONSTANT_2 * y * z,
        BASIS_CONSTANT_3 * (3.0 * z * z - 1.0),
        BASIS_CONSTANT_2 * x * z,
        BASIS_CONSTANT_4 * (x * x - y * y)
    });
}

} // anonymous namespace

ShColorCoeffs projectEnvironment(ConstImagePtr env, bool irradiance)
{
    ShColorCoeffs shCoeffs;

    for (unsigned int y = 0; y < env->getHeight(); y++)
    {
        double theta = imageYToTheta(y, env->getHeight());
        double texelWeight = texelSolidAngle(y, env->getWidth(), env->getHeight());

        for (unsigned int x = 0; x < env->getWidth(); x++)
        {
            // Sample the color at these coordinates.
            Color4 color = env->getTexelColor(x, y);

            // Compute the direction vector.
            double phi = imageXToPhi(x, env->getWidth());
            Vector3d dir = sphericalToCartesian(theta, phi);

            // Evaluate the given direction as SH coefficients.
            ShScalarCoeffs shDir = evalDirection(dir);

            // Combine color with texel weight.
            Color3d weightedColor(color[0] * texelWeight,
                                  color[1] * texelWeight,
                                  color[2] * texelWeight);

            // Update coefficients for the influence of this texel.
            for (size_t i = 0; i < shCoeffs.NUM_COEFFS; i++)
            {
                shCoeffs[i] += weightedColor * shDir[i];
            }
        }
    }

    // If irradiance is requested, then apply constant factors to convolve the
    // signal by a clamped cosine lobe.
    if (irradiance)
    {
        shCoeffs[0] *= COSINE_CONSTANT_0;
        shCoeffs[1] *= COSINE_CONSTANT_1;
        shCoeffs[2] *= COSINE_CONSTANT_1;
        shCoeffs[3] *= COSINE_CONSTANT_1;
        shCoeffs[4] *= COSINE_CONSTANT_2;
        shCoeffs[5] *= COSINE_CONSTANT_2;
        shCoeffs[6] *= COSINE_CONSTANT_2;
        shCoeffs[7] *= COSINE_CONSTANT_2;
        shCoeffs[8] *= COSINE_CONSTANT_2;
    }

    return shCoeffs;
}

ImagePtr renderEnvironment(ShColorCoeffs coeffs, unsigned int width, unsigned int height)
{
    ImagePtr env = Image::create(width, height, 3, Image::BaseType::FLOAT);
    env->createResourceBuffer();

    for (unsigned int y = 0; y < env->getHeight(); y++)
    {
        double theta = imageYToTheta(y, env->getHeight());
        for (unsigned int x = 0; x < env->getWidth(); x++)
        {
            // Compute the direction vector.
            double phi = imageXToPhi(x, env->getWidth());
            Vector3d dir = sphericalToCartesian(theta, phi);

            // Evaluate the given direction as SH coefficients.
            ShScalarCoeffs shDir = evalDirection(dir);

            // Compute the signal color in this direction.
            Color3d color;
            for (size_t i = 0; i < coeffs.NUM_COEFFS; i++)
            {
                color += coeffs[i] * shDir[i];
            }

            // Store the color as an environment texel.
            env->setTexelColor(x, y, Color4((float) color[0], (float) color[1], (float) color[2], 1.0f));
        }
    }

    return env;
}

} // namespace MaterialX
