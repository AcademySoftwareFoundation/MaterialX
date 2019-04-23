//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_VIEWHANDLER_H
#define MATERIALX_VIEWHANDLER_H

/// @file
/// Utility for providing view data

#include <MaterialXCore/Types.h>
#include <memory>

namespace MaterialX
{

/// Shared pointer to a ViewHandler
using ViewHandlerPtr = std::shared_ptr<class ViewHandler>;

/// @class ViewHandler
/// Utility view handler for creating and providing 
/// View data for shader binding.
///
class ViewHandler
{
  public:
    /// Static instance create function
    static ViewHandlerPtr create() { return std::make_shared<ViewHandler>(); }

    /// Default constructor
    ViewHandler() {};
    
    /// Default destructor
    virtual ~ViewHandler() {};

    /// Create a view matrix given a eye position, a target position and an up vector
    static Matrix44 createViewMatrix(const Vector3& eye,
                                     const Vector3& target,
                                     const Vector3& up);

    /// Create a perpective matrix given a set of clip planes
    static Matrix44 createPerspectiveMatrix(float left, float right,
                                            float bottom, float top,
                                            float nearP, float farP);

    /// Set the world matrix
    void setWorldMatrix(Matrix44& m)
    {
        _worldMatrix = m;
    }

    /// Get the projection matrix
    Matrix44& projectionMatrix()
    {
        return _projectionMatrix;
    }

    /// Get the view matrix
    Matrix44& viewMatrix()
    {
        return _viewMatrix;
    }

    /// Get the view position
    Vector3& viewPosition()
    {
        return _viewPosition;
    }

    /// Get the view direction
    Vector3& viewDirection()
    {
        return _viewDirection;
    }

    /// Get the world matrix
    Matrix44& worldMatrix()
    {
        return _worldMatrix;
    }

    /// @}
    /// @name General utilities
    /// @{

    /// Convert from degress to radians
    /// @param degrees Degree value
    /// @return value converted to radians
    float degreesToRadians(float degrees) const;

    /// PI
    static float PI_VALUE;

    /// @}

  protected:
    /// World matrix
    Matrix44 _worldMatrix;
    /// View matrix
    Matrix44 _viewMatrix;
    /// View position
    Vector3 _viewPosition;
    /// View direction
    Vector3 _viewDirection;
    /// Projection matrix
    Matrix44 _projectionMatrix;
};

} // namespace MaterialX

#endif
