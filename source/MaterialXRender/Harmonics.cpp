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

Sh3ScalarCoeffs evalDirection(const Vector3d& dir)
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

    return Sh3ScalarCoeffs(
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

Sh3ColorCoeffs projectEnvironment(ConstImagePtr env, bool irradiance)
{
    Sh3ColorCoeffs shEnv;

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
            Sh3ScalarCoeffs shDir = evalDirection(dir);

            // Combine color with texel weight.
            Color3d weightedColor(color[0] * texelWeight,
                                  color[1] * texelWeight,
                                  color[2] * texelWeight);

            // Update coefficients for the influence of this texel.
            for (size_t i = 0; i < shEnv.NUM_COEFFS; i++)
            {
                shEnv[i] += weightedColor * shDir[i];
            }
        }
    }

    // If irradiance is requested, then apply constant factors to convolve the
    // signal by a clamped cosine kernel.
    if (irradiance)
    {
        shEnv[0] *= COSINE_CONSTANT_0;
        shEnv[1] *= COSINE_CONSTANT_1;
        shEnv[2] *= COSINE_CONSTANT_1;
        shEnv[3] *= COSINE_CONSTANT_1;
        shEnv[4] *= COSINE_CONSTANT_2;
        shEnv[5] *= COSINE_CONSTANT_2;
        shEnv[6] *= COSINE_CONSTANT_2;
        shEnv[7] *= COSINE_CONSTANT_2;
        shEnv[8] *= COSINE_CONSTANT_2;
    }

    return shEnv;
}

ImagePtr renderEnvironment(const Sh3ColorCoeffs& shEnv, unsigned int width, unsigned int height)
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
            Sh3ScalarCoeffs shDir = evalDirection(dir);

            // Compute the signal color in this direction.
            Color3d signalColor;
            for (size_t i = 0; i < shEnv.NUM_COEFFS; i++)
            {
                signalColor += shEnv[i] * shDir[i];
            }

            // Clamp the color and store as an environment texel.
            Color4 outputColor(
                (float) std::max(signalColor[0], 0.0),
                (float) std::max(signalColor[1], 0.0),
                (float) std::max(signalColor[2], 0.0),
                1.0f);
            env->setTexelColor(x, y, outputColor);
        }
    }

    return env;
}

} // namespace MaterialX
