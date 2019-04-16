//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/ViewHandler.h>
#include <cmath>

namespace MaterialX
{ 

float ViewHandler::PI_VALUE = 3.14159265358979323846f;

float ViewHandler::degreesToRadians(float degrees) const
{
    return (degrees * PI_VALUE / 180.0f);
}

void ViewHandler::setPerspectiveProjectionMatrix(float fov,
                                                 float aspectRatio,
                                                 float nearClipPlane,
                                                 float farClipPlane)
{
    for (unsigned int i = 0; i < 4; i++)
    {
        for (unsigned int j = 0; j < 4; j++)
        {
            _projectionMatrix[i][j] = 0.0f;
        }
    }

    float scaley = 1.0f / std::tan(degreesToRadians(fov / 2.0f));
    float scalex = scaley / aspectRatio;
    float clipDistance = farClipPlane - nearClipPlane;

    _projectionMatrix[0][0] = scalex;
    _projectionMatrix[1][1] = scaley;
    _projectionMatrix[2][2] = -(nearClipPlane + farClipPlane) / clipDistance;
    _projectionMatrix[2][3] = -1;
    _projectionMatrix[3][2] = -((2.0f * nearClipPlane * farClipPlane) / clipDistance);
}

void ViewHandler::setOrthoGraphicProjectionMatrix(float left,
                                                  float right,
                                                  float bottom,
                                                  float top,
                                                  float nearClipPlane,
                                                  float farClipPlane)
{
    for (unsigned int i = 0; i < 4; i++)
    {
        for (unsigned int j = 0; j < 4; j++)
        {
            _projectionMatrix[i][j] = 0.0f;
        }
    }

    float clipDistance = farClipPlane - nearClipPlane;

    _projectionMatrix[0][0] = 2.0f / (right - left);
    _projectionMatrix[1][1] = 2.0f / (top - bottom);
    _projectionMatrix[2][2] = -2.0f / clipDistance;
    _projectionMatrix[3][0] = -(right + left) / (right - left);
    _projectionMatrix[3][1] = -(top + bottom) / (top - bottom);
    _projectionMatrix[3][2] = -(farClipPlane + nearClipPlane) / clipDistance;
    _projectionMatrix[3][3] = 1.0f;
}

}
