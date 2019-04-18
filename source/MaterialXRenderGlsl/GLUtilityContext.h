//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_GLUTILITYCONTEXT_H
#define MATERIALX_GLUTILITYCONTEXT_H

/// @file
/// OpenGL utility context

#include <MaterialXRenderHw/Window/WindowWrapper.h>
#include <memory>

#if defined(_WIN32)
#include <MaterialXRenderHw/Window/SimpleWindow.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#elif defined(__linux__)
#include <MaterialXRenderGlsl/External/GLew/glxew.h>
#endif

namespace MaterialX
{
/// Platform dependent definition of a hardware context
#if defined(_WIN32)
using HardwareContextHandle = HGLRC;
#elif defined(__linux__)
using HardwareContextHandle = GLXContext;
#elif defined(__APPLE__)
using HardwareContextHandle = void*;
#else
using HardwareContextHandle = void*;
#endif

/// GLUtilityContext shared pointer
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

#if defined(__linux__)
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

#if defined(_WIN32)
    /// Share this context with an external one
    void shareLists(HardwareContextHandle context);
#endif

  protected:
    /// Create the base context. A OpenGL context to share with can be passed in.
    GLUtilityContext(const WindowWrapper& windowWrapper, HardwareContextHandle context = 0);

#if defined(_WIN32)
    /// Offscreen window required for context operations
    SimpleWindow _dummyWindow;
#elif defined(__linux__)
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
