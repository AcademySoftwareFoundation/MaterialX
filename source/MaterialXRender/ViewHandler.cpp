//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/ViewHandler.h>

namespace MaterialX
{ 

const float PI = std::acos(-1.0f);

float ViewHandler::degreesToRadians(float degrees) const
{
    return (degrees * PI / 180.0f);
}

Matrix44 ViewHandler::createViewMatrix(const Vector3& eye,
                                       const Vector3& target,
                                       const Vector3& up)
{
    Vector3 z = (target - eye).getNormalized();
    Vector3 x = z.cross(up).getNormalized();
    Vector3 y = x.cross(z);

    return Matrix44(
        x[0], x[1], x[2], -x.dot(eye),
        y[0], y[1], y[2], -y.dot(eye),
        -z[0], -z[1], -z[2], z.dot(eye),
        0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix44 ViewHandler::createPerspectiveMatrix(float left, float right,
                                              float bottom, float top,
                                              float nearP, float farP)
{
    return Matrix44(
        (2.0f * nearP) / (right - left), 0.0f, (right + left) / (right - left), 0.0f,
        0.0f, (2.0f * nearP) / (top - bottom), (top + bottom) / (top - bottom), 0.0f,
        0.0f, 0.0f, -(farP + nearP) / (farP - nearP), -(2.0f * farP * nearP) / (farP - nearP),
        0.0f, 0.0f, -1.0f, 0.0f);
}

} // namespace MaterialX
