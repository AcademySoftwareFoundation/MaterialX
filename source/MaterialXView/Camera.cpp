#include <MaterialXView/Camera.h>

#include <string.h>

namespace mx = MaterialX;

struct Camera::Internal
{
    Internal()
    : mActive(false), mLastPos(0.f, 0.f), mSize(0.f, 0.f),
      mQuat(mx::Quaternion::IDENTITY),
      mIncr(mx::Quaternion::IDENTITY),
      mSpeedFactor(2.f) { }

    void setSize(mx::Vector2 size) {
        mSize = size;
    }

    void button(mx::Vector2 pos, bool pressed) {
        mActive = pressed;
        mLastPos = pos;
        if (!mActive)
            mQuat = (mIncr * mQuat).getNormalized();
        mIncr = mx::Quaternion::IDENTITY;
    }

    bool motion(mx::Vector2 pos) {
        if (!mActive)
            return false;

        /* Based on the rotation controller from AntTweakBar */
        float invMinDim = 1.0f / (mSize[0] < mSize[1] ? mSize[0] : mSize[1]);
        float w = mSize[0], h = mSize[1];

        float ox = (mSpeedFactor * (2*mLastPos[0]  - w) + w) - w - 1.0f;
        float tx = (mSpeedFactor * (2*pos[0]       - w) + w) - w - 1.0f;
        float oy = (mSpeedFactor * (h - 2*mLastPos[1])  + h) - h - 1.0f;
        float ty = (mSpeedFactor * (h - 2*pos[1])       + h) - h - 1.0f;

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
            mIncr = mx::Quaternion::createFromAxisAngle(axis, angle);
            if (!std::isfinite(mIncr.getMagnitude()))
                mIncr = mx::Quaternion::IDENTITY;
        }
        return true;
    }

    mx::Matrix44 matrix() const {
        return mx::Matrix44::createRotation(mIncr * mQuat);
    }

    /// Whether or not this Arcball is currently active.
    bool mActive;

    /// The last click position (which triggered the Arcball to be active / non-active).
    mx::Vector2 mLastPos;

    /// The size of this Arcball.
    mx::Vector2 mSize;

    /**
     * The current stable state.  When this Arcball is active, represents the
     * state of this Arcball when \ref Arcball::button was called with
     * ``down = true``.
     */
    mx::Quaternion mQuat;

    /// When active, tracks the overall update to the state.  Identity when non-active.
    mx::Quaternion mIncr;

    /**
     * The speed at which this Arcball rotates.  Smaller values mean it rotates
     * more slowly, higher values mean it rotates more quickly.
     */
    float mSpeedFactor;
};

Camera::Camera()
    : mInternal(new Internal()) {}

Camera::~Camera() {
    delete mInternal;
}

Camera& Camera::operator= (const Camera& c)
{
    memcpy(mInternal, c.mInternal, sizeof(Internal));
    return *this;
}

// set the size of a virtual sphere for click-drag interaction
void Camera::setSize(mx::Vector2 size) {
    mInternal->setSize(size);
}

void Camera::button(mx::Vector2 pos, bool pressed) {
    mInternal->button(pos, pressed);
}

bool Camera::motion(mx::Vector2 pos) {
    return mInternal->motion(pos);
}

mx::Matrix44 Camera::matrix() const {
    return mInternal->matrix();
}
