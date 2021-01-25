#include <MaterialXView/Camera.h>

#include <string.h>

namespace mx = MaterialX;

class Camera::Internal
{
  public:
    Internal() :
        _active(false),
        _lastPos(0.f, 0.f),
        _size(0.f, 0.f),
        _quat(mx::Quaternion::IDENTITY),
        _incr(mx::Quaternion::IDENTITY),
        _speedFactor(2.f)
    {
    }

    void setSize(mx::Vector2 size)
    {
        _size = size;
    }

    void button(mx::Vector2 pos, bool pressed)
    {
        _active = pressed;
        _lastPos = pos;
        if (!_active)
            _quat = (_incr * _quat).getNormalized();
        _incr = mx::Quaternion::IDENTITY;
    }

    bool motion(mx::Vector2 pos)
    {
        if (!_active)
        {
            return false;
        }

        /* Based on the rotation controller from AntTweakBar */
        float invMinDim = 1.0f / (_size[0] < _size[1] ? _size[0] : _size[1]);
        float w = _size[0], h = _size[1];

        float ox = (_speedFactor * (2*_lastPos[0]  - w) + w) - w - 1.0f;
        float tx = (_speedFactor * (2*pos[0]       - w) + w) - w - 1.0f;
        float oy = (_speedFactor * (h - 2*_lastPos[1])  + h) - h - 1.0f;
        float ty = (_speedFactor * (h - 2*pos[1])       + h) - h - 1.0f;

        ox *= invMinDim; oy *= invMinDim;
        tx *= invMinDim; ty *= invMinDim;

        mx::Vector3 v0(ox, oy, 1.0f), v1(tx, ty, 1.0f);
        if (v0.dot(v0) > 1e-4f && v1.dot(v1) > 1e-4f) {
            v0 = v0.getNormalized(); 
            v1 = v1.getNormalized();
            mx::Vector3 axis = v0.cross(v1);
            float sa = std::sqrt(axis.dot(axis)),
                  ca = v0.dot(v1),
                  angle = std::atan2(sa, ca);
            if (tx*tx + ty*ty > 1.0f)
                angle *= 1.0f + 0.2f * (std::sqrt(tx*tx + ty*ty) - 1.0f);
            axis = axis.getNormalized();
            _incr = mx::Quaternion::createFromAxisAngle(axis, angle);
            if (!std::isfinite(_incr.getMagnitude()))
                _incr = mx::Quaternion::IDENTITY;
        }
        return true;
    }

    mx::Matrix44 matrix() const
    {
        return mx::Matrix44::createRotation(_incr * _quat);
    }

  public:
    // Whether or not this camera is currently active.
    bool _active;

    // The last click position (which triggered the camera to be active / non-active).
    mx::Vector2 _lastPos;

    // The size of this Arcball.
    mx::Vector2 _size;

    // The current stable state.  When this camera is active, represents the state
    // of this camera when the button method was called with pressed set to true.
    mx::Quaternion _quat;

    /// When active, tracks the overall update to the state.  Identity when non-active.
    mx::Quaternion _incr;

    // The speed at which this Arcball rotates.  Smaller values mean it rotates
    // more slowly, higher values mean it rotates more quickly.
    float _speedFactor;
};

Camera::Camera() :
    _internal(new Internal())
{
}

Camera::~Camera()
{
    delete _internal;
}

Camera& Camera::operator=(const Camera& c)
{
    memcpy(_internal, c._internal, sizeof(Internal));
    return *this;
}

void Camera::setSize(mx::Vector2 size)
{
    _internal->setSize(size);
}

void Camera::button(mx::Vector2 pos, bool pressed)
{
    _internal->button(pos, pressed);
}

bool Camera::motion(mx::Vector2 pos)
{
    return _internal->motion(pos);
}

mx::Matrix44 Camera::matrix() const
{
    return _internal->matrix();
}
