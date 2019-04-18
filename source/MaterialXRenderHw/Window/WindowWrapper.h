//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_WINDOWWRAPPER_H
#define MATERIALX_WINDOWWRAPPER_H

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <X11/X.h> // for Window
#include <X11/Xlib.h> // for Display
using Widget = struct _WidgetRec*;
#endif

namespace MaterialX
{
/// OS specific type windowing definitions
#if defined(_WIN32)
/// External handle is a window handle
using ExternalWindowHandle = HWND;
/// Internal handle is a device context
using InternalWindowHandle = HDC;
/// Display handle concept has no equivalence on Windows
using DisplayHandle = void*;
#elif defined(__linux__)
/// External handle is a widget
using ExternalWindowHandle = Widget;
/// Internal handle is the window for the widget
using InternalWindowHandle = Window;
/// Display handle is the X display
using DisplayHandle = Display*;
/// Application shell
using Widget = struct _WidgetRec*;
#elif defined(__APPLE__)
/// External handle is a window handle
using ExternalWindowHandle = void*;
/// Internal handle concept has no equivalence on Mac
using InternalWindowHandle = void*;
/// Display handle concept has no equivalence on Mac
using DisplayHandle = void*;
#else
using Widget = void*;
using ExternalWindowHandle = void*;
using InternalWindowHandle = void*;
using DisplayHandle = void*;
#endif

///
/// @class WindowWrapper
/// Generic wrapper for encapsulating a "window" construct
/// Each supported platform will have specific storage and management logic.
///
class WindowWrapper
{
  public:
    /// Default constructor
    WindowWrapper();

    /// Default destructor
    virtual ~WindowWrapper();

    /// Construct a wrapper using windowing information
#if defined(__linux__)
    WindowWrapper(ExternalWindowHandle externalHandle, InternalWindowHandle internalHandle = 0,
                  DisplayHandle display = 0);
#else
    WindowWrapper(ExternalWindowHandle externalHandle, InternalWindowHandle internalHandle = nullptr,
                  DisplayHandle display = 0);
#endif

    /// Copy constructor
    WindowWrapper(const WindowWrapper& other);

    /// Assignment operator
    const WindowWrapper& operator=(const WindowWrapper& other);

    /// Return "external" handle
    ExternalWindowHandle externalHandle() const
    {
        return _externalHandle;
    }

    /// Return "internal" handle
    InternalWindowHandle internalHandle() const
    {
        return _internalHandle;
    }

    /// Check that there is a valid OS handle set.
    /// It is sufficient to just check the internal handle
    bool isValid() const
    {
        return _internalHandle != 0;
    }

#if defined(__linux__)
    /// Return frame buffer X window
    Window getFrameBufferWindow() const
    {
        return _framebufferWindow;
    }

    /// Set frame buffer X window
    void setFrameBufferWindow(Window window)
    {
        _framebufferWindow = window;
    }

    /// Rreturn X display
    Display* getDisplay() const
    {
        return _display;
    }
#endif
    /// Release resources stored in wrapper
    void release();

  protected:
    ExternalWindowHandle _externalHandle;
    InternalWindowHandle _internalHandle;

#if defined(__linux__)
    /// Window ID of framebuffer instance created in the wrapper
    Window _framebufferWindow;
    /// X Display
    Display* _display;
#endif

};

} // namespace MaterialX

#endif
