#include <MaterialXView/Camera.h>

namespace mx = MaterialX;

void Camera::buttonEvent(const mx::Vector2& pos, bool pressed)
{
    _active = pressed;
    _lastPos = pos;
    if (!_active)
    {
        _quat = (_incr * _quat).getNormalized();
    }
    _incr = mx::Quaternion::IDENTITY;
}

bool Camera::applyMotion(const mx::Vector2& pos)
{
    if (!_active)
    {
        return false;
    }

    float invMinDim = 1.0f / (_size[0] < _size[1] ? _size[0] : _size[1]);
    float w = _size[0], h = _size[1];

    float ox = (_speedFactor * (2*_lastPos[0]  - w) + w) - w - 1.0f;
    float tx = (_speedFactor * (2*pos[0]       - w) + w) - w - 1.0f;
    float oy = (_speedFactor * (h - 2*_lastPos[1])  + h) - h - 1.0f;
    float ty = (_speedFactor * (h - 2*pos[1])       + h) - h - 1.0f;

    ox *= invMinDim; oy *= invMinDim;
    tx *= invMinDim; ty *= invMinDim;

    mx::Vector3 v0(ox, oy, 1.0f), v1(tx, ty, 1.0f);
    if (v0.dot(v0) > 1e-4f && v1.dot(v1) > 1e-4f)
    {
        v0 = v0.getNormalized(); 
        v1 = v1.getNormalized();
        mx::Vector3 axis = v0.cross(v1);
        float sa = std::sqrt(axis.dot(axis));
        float ca = v0.dot(v1);
        float angle = std::atan2(sa, ca);
        if (tx*tx + ty*ty > 1.0f)
        {
            angle *= 1.0f + 0.2f * (std::sqrt(tx*tx + ty*ty) - 1.0f);
        }
        axis = axis.getNormalized();
        _incr = mx::Quaternion::createFromAxisAngle(axis, angle);
        if (!std::isfinite(_incr.getMagnitude()))
        {
            _incr = mx::Quaternion::IDENTITY;
        }
    }
    return true;
}

mx::Matrix44 Camera::matrix() const
{
    return mx::Matrix44::createRotation(_incr * _quat);
}
