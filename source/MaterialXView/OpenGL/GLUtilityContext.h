#ifndef MATERIALX_GLUTILITYCONTEXT_H
#define MATERIALX_GLUTILITYCONTEXT_H

#include <MaterialXView/Window/HardwarePlatform.h>
#include <MaterialXView/Window/WindowWrapper.h>
#include <memory>

#if defined(OSWin_)
#include <MaterialXView/Window/SimpleWindow.h>
#elif defined(OSMac_)
#include <OpenGL/gl.h>
#elif defined(OSLinux_)
#include <MaterialXView/External/GLew/glxew.h>
#endif

namespace MaterialX
{
/// Platform dependent definition of a hardware context
#if defined(OSWin_)
using HardwareContextHandle = HGLRC;
#elif defined(OSLinux_)
using HardwareContextHandle = GLXContext;
#elif defined(OSMac_)
using HardwareContextHandle = void*;
#else
using HardwareContextHandle = void*;
#endif

// GLUtilityContext shared pointer
using GLUtilityContextPtr = std::shared_ptr<class GLUtilityContext>;

/// @class GLUtilityContext
/// Base OpenGL context singleton. 
/// Used as a utility context to perform OpenGL operations from,
/// and context for resource sharing between contexts.
///
class GLUtilityContext
{
  public:
    
    /// Create a utility context 
    static GLUtilityContextPtr create(const WindowWrapper& windowWrapper, HardwareContextHandle context = 0);

    /// Default destructor
    virtual ~GLUtilityContext();

    /// Return OpenGL context handle
    HardwareContextHandle contextHandle() const
    {
        return _contextHandle;
    }

#if defined(OSLinux_)
    /// Return X display associated with context
    Display *display() const { return _display; }
#endif

    /// Return if context is valid
    bool isValid() const
    {
        return _isValid;
    }

    /// Make the context "current" before execution of OpenGL operations
    int makeCurrent();

#if defined(OSWin_)
    /// Share this context with an external one
    void shareLists(HardwareContextHandle context);
#endif

  protected:
    /// Create the base context. A OpenGL context to share with can be passed in.
    GLUtilityContext(const WindowWrapper& windowWrapper, HardwareContextHandle context = 0);

#if defined(OSWin_)
    /// Offscreen window required for context operations
    SimpleWindow _dummyWindow;
#elif defined(OSLinux_)
    /// Offscreen window required for context operations
    Window _dummyWindow;
    /// X Display used by context operations
    Display *_display;
    /// Window wrapper used by context operations
    WindowWrapper _windowWrapper;
#endif

    /// Context handle
    HardwareContextHandle _contextHandle;

    /// Flag to indicate validity
    bool _isValid;
};

} // namespace MaterialX

#endif
