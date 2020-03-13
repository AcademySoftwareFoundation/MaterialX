//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRenderHw/WindowWrapper.h>

#if defined(__linux__)
#include <X11/Intrinsic.h>
#elif defined(__APPLE__)
#include <MaterialXRenderHw/WindowCocoaWrappers.h>
#endif

namespace MaterialX
{

#if defined(_WIN32)
//
// Window platform code
//

WindowWrapper::WindowWrapper() :
    _externalHandle(0),
    _internalHandle(0)
{
}

WindowWrapper::WindowWrapper(ExternalWindowHandle externalHandle,
                             InternalWindowHandle internalHandle,
                             DisplayHandle /*display*/)
{
    _externalHandle = externalHandle;
    if (_externalHandle && !internalHandle)
    {
        // Cache a HDC that corresponds to the window handle.
        _internalHandle = GetDC(_externalHandle);
    }
    else
    {
        _internalHandle = internalHandle;
    }
}

WindowWrapper::WindowWrapper(const WindowWrapper& other)
{
    _externalHandle = other._externalHandle;
    if (_externalHandle && !other._internalHandle)
    {
        // Cache a HDC that corresponds to the window handle
        _internalHandle = GetDC(_externalHandle);
    }
    else
    {
        _internalHandle = other._internalHandle;
    }
}

const WindowWrapper& WindowWrapper::operator=(const WindowWrapper& other)
{
    if (this == &other)
    {
        return *this;
    }

    release();

    _externalHandle = other._externalHandle;
    if (_externalHandle && !other._internalHandle)
    {
        // Cache a HDC that corresponds to the window handle
        _internalHandle = GetDC(_externalHandle);
    }
    else
    {
        _internalHandle = other._internalHandle;
    }

    return *this;
}

WindowWrapper::~WindowWrapper()
{
    release();
}

void WindowWrapper::release()
{
    if (_externalHandle)
    {
        // Release acquired DC
        ReleaseDC(_externalHandle, _internalHandle);
    }
    _externalHandle = 0;
    _internalHandle = 0;
}

#elif defined(__linux__)
//
// Linux (X-specific) code
//

// Default constructor.
WindowWrapper::WindowWrapper() :
    _externalHandle(0),
    _internalHandle(0),
    _framebufferWindow(0),
    _display(0)
{
}

WindowWrapper::WindowWrapper(ExternalWindowHandle externalHandle,
                             InternalWindowHandle internalHandle,
                             DisplayHandle display)
{
    _display = display;
    _framebufferWindow = 0;
    _externalHandle = externalHandle;
    // Cache a pointer to the window.
    if (internalHandle)
        _internalHandle = internalHandle;
    else
        _internalHandle = XtWindow(externalHandle);
}

WindowWrapper::WindowWrapper(const WindowWrapper& other)
{
    _framebufferWindow = other._framebufferWindow;
    _externalHandle = other._externalHandle;
    _internalHandle = other._internalHandle;
    _display = other._display;
}

const WindowWrapper& WindowWrapper::operator=(const WindowWrapper& other)
{
    _framebufferWindow = other._framebufferWindow;
    _externalHandle = other._externalHandle;
    _internalHandle = other._internalHandle;
    _display = other._display;
    return *this;
}

WindowWrapper::~WindowWrapper()
{
    release();
}

void WindowWrapper::release()
{
    // No explicit release calls are required.
    _externalHandle = 0;
    _internalHandle = 0;
    _framebufferWindow = 0;
    _display = 0;
}

#elif defined(__APPLE__)
//
// OSX (Apple) specific code
//

WindowWrapper::WindowWrapper() :
    _externalHandle(0),
    _internalHandle(0)
{
}

WindowWrapper::WindowWrapper(ExternalWindowHandle externalHandle,
                             InternalWindowHandle internalHandle,
                             DisplayHandle display)
{
    _externalHandle = externalHandle;
    // Cache a pointer to the window.
    _internalHandle = NSUtilGetView(externalHandle);
}

WindowWrapper::WindowWrapper(const WindowWrapper& other)
{
    _externalHandle = other._externalHandle;
    _internalHandle = NSUtilGetView(_externalHandle);
}

const WindowWrapper& WindowWrapper::operator=(const WindowWrapper& other)
{
    _externalHandle = other._externalHandle;
    _internalHandle = NSUtilGetView(_externalHandle);
    return *this;
}

WindowWrapper::~WindowWrapper()
{
    release();
}

void WindowWrapper::release()
{
    // No explicit release calls are required.
    _externalHandle = 0;
    _internalHandle = 0;
}
#endif

} // namespace MaterialX
