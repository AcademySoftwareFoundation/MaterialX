//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_VIEWHANDLER_H
#define MATERIALX_VIEWHANDLER_H

#include <MaterialXCore/Types.h>
#include <memory>

namespace MaterialX
{

/// Shared pointer to a ViewHandler
using ViewHandlerPtr = std::shared_ptr<class ViewHandler>;

/// @class @ViewHandler
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

    /// @name View matrices and vectors
    /// @{

    /// Set a matrix to a perspective projection
    /// @param fov Field of view
    /// @param aspectRatio Aspect ration (viewport width /  viewport height)
    /// @param nearClipPlane Near clip plane
    /// @param farClipPlane Far clip plane
    void setPerspectiveProjectionMatrix(float fov,
                                        float aspectRatio,
                                        float nearClipPlane,
                                        float farClipPlane);

    /// Set a matrix to an orthographic projection
    /// @param left Left clip plane
    /// @param right Right clip plane
    /// @param bottom Bottom clip plane
    /// @param top Top clip plane
    /// @param nearClipPlane Near clip plane
    /// @param farClipPlane Far clip plane
    void setOrthoGraphicProjectionMatrix(float left,
                                         float right,
                                         float bottom,
                                         float top,
                                         float nearClipPlane,
                                         float farClipPlane);

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
