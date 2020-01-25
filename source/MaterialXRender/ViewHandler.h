//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_VIEWHANDLER_H
#define MATERIALX_VIEWHANDLER_H

/// @file
/// Utility for providing view data

#include <MaterialXCore/Types.h>

namespace MaterialX
{

/// Shared pointer to a ViewHandler
using ViewHandlerPtr = std::shared_ptr<class ViewHandler>;

/// @class ViewHandler
/// Utility view handler for creating and providing 
/// View data for shader binding.
class ViewHandler
{
  public:
    ViewHandler()
    {
    }
    virtual ~ViewHandler() { }

    /// @name Utility Functions
    /// @{

    /// Create a new view handler.
    static ViewHandlerPtr create() { return std::make_shared<ViewHandler>(); }

    /// Create a view matrix given an eye position, a target position and an up vector.
    static Matrix44 createViewMatrix(const Vector3& eye,
                                     const Vector3& target,
                                     const Vector3& up);

    /// Create a perpective projection matrix given a set of clip planes.
    static Matrix44 createPerspectiveMatrix(float left, float right,
                                            float bottom, float top,
                                            float nearP, float farP);

    /// Create an orthographic projection matrix given a set of clip planes.
    static Matrix44 createOrthographicMatrix(float left, float right,
                                             float bottom, float top,
                                             float nearP, float farP);
    /// @}

  public:
    Matrix44 worldMatrix;
    Matrix44 viewMatrix;
    Vector3 viewPosition;
    Vector3 viewDirection;
    Matrix44 projectionMatrix;
};

} // namespace MaterialX

#endif
