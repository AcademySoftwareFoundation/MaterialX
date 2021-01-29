
#ifndef MATERIALXVIEW_CAMERA_H
#define MATERIALXVIEW_CAMERA_H

#include <MaterialXCore/Types.h>

namespace mx = MaterialX;

/// A viewer camera class, based on the ArcBall class in NanoGUI.
class Camera
{
  public:
    Camera() :
        _active(false),
        _lastPos(0.0f, 0.0f),
        _size(0.0f, 0.0f),
        _quat(mx::Quaternion::IDENTITY),
        _incr(mx::Quaternion::IDENTITY),
        _speedFactor(2.f)
    {
    }
    ~Camera() { };

    // Set the size of a virtual window for click-drag interaction.
    void setSize(const mx::Vector2& size)
    {
        _size = size;
    }

    // Indicates a button state change, with pos being the instantaneous location of the mouse.
    void buttonEvent(const mx::Vector2& pos, bool pressed);

    // Mouse motion is continuously supplied here.
    bool applyMotion(const mx::Vector2& pos);

    // Current view matrix.
    mx::Matrix44 matrix() const;

  protected:
    // Whether or not this camera is currently active.
    bool _active;

    // The last click position (which triggered the camera to be active / non-active).
    mx::Vector2 _lastPos;

    // The size of this camera.
    mx::Vector2 _size;

    // The current stable state.  When this camera is active, represents the state
    // of this camera when the button method was called with pressed set to true.
    mx::Quaternion _quat;

    // When active, tracks the overall update to the state.  Identity when non-active.
    mx::Quaternion _incr;

    // The speed at which this camera rotates.  Smaller values mean it rotates
    // more slowly, higher values mean it rotates more quickly.
    float _speedFactor;
};

#endif
