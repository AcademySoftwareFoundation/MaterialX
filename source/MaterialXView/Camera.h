
#ifndef MATERIALXVIEW_CAMERA_H
#define MATERIALXVIEW_CAMERA_H

#include <MaterialXCore/Types.h>

struct Camera
{
    Camera();
    ~Camera();

    Camera& operator= (const Camera&);

    // set the size of a virtual sphere for click-drag interaction
    void setSize(MaterialX::Vector2 size);

    // indicates a button state change, with pos being the instantaneous location of the mouse
    void button(MaterialX::Vector2 pos, bool pressed);

    // mouse motion is continuously supplied here
    bool motion(MaterialX::Vector2 pos);

    // current view matrix
    MaterialX::Matrix44 matrix() const;

  protected:
    class Internal;
    Internal* _internal;
};

#endif
