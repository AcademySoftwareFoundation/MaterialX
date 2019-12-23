//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#if defined(_WIN32)
#include <windows.h> // For Windows calls

#elif defined(__linux__)
#include <dlfcn.h> // For dlopen
#include <MaterialXRenderGlsl/External/GLew/glxew.h>
#include <X11/Intrinsic.h>

#elif defined(__APPLE__)
#include <MaterialXRenderHw/WindowCocoaWrappers.h>
#include <MaterialXRenderGlsl/External/GLew/glew.h>
#include <MaterialXRenderGlsl/GLCocoaWrappers.h>
#endif

#include <MaterialXRenderGlsl/External/GLew/glew.h>
#include <MaterialXRenderGlsl/GLUtilityContext.h>

namespace MaterialX
{
#if defined(_WIN32)
//
// Windows implementation
//
GLUtilityContext::GLUtilityContext(const WindowWrapper& /*windowWrapper*/, HardwareContextHandle sharedWithContext) :
    _contextHandle(nullptr),
    _isValid(false)
{
    // For windows, we need a HDC to create an OpenGL context.
    // Create a dummy 1x1 window and use it' HDC.
    _dummyWindow.initialize("__GL_BASE_CONTEXT_DUMMY_WINDOW__", 1, 1, nullptr);
    WindowWrapper dummyWindowWrapper = _dummyWindow.windowWrapper();

    if (dummyWindowWrapper.isValid())
    {
        // Use a generic pixel format to create the context
        static PIXELFORMATDESCRIPTOR pfd =
        {
            sizeof(PIXELFORMATDESCRIPTOR),
            1,
            PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
            PFD_TYPE_RGBA,
            32,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0,
            16, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
        };

        int chosenPixelFormat = ChoosePixelFormat(dummyWindowWrapper.internalHandle(), &pfd);
        if (chosenPixelFormat)
        {
            if (SetPixelFormat(dummyWindowWrapper.internalHandle(), chosenPixelFormat, &pfd))
            {
                _contextHandle = wglCreateContext(dummyWindowWrapper.internalHandle());
                if (_contextHandle)
                {
                    if (sharedWithContext)
                    {
                        shareLists(sharedWithContext);
                    }

                    int makeCurrentOk = wglMakeCurrent(dummyWindowWrapper.internalHandle(), _contextHandle);
                    if (makeCurrentOk)
                    {
                        _isValid = true;
                    }
                }
            }
        }
    }
}

void GLUtilityContext::shareLists(HardwareContextHandle context)
{
    if (_isValid)
    {
        wglShareLists(_contextHandle, context);
    }
}

#elif defined(__linux__)
//
// Linux context implementation
//
GLUtilityContext::GLUtilityContext(const WindowWrapper& windowWrapper,
    HardwareContextHandle sharedWithContext)
{
    _isValid = false;

    //
    // Unix implementation
    //
    _windowWrapper = windowWrapper;

    // Get connection to X Server
    _display = windowWrapper.getDisplay();

    // Load in OpenGL library
    void *libHandle = dlopen("libGL.so", RTLD_LAZY);

    //
    // Get X required functions
    //
    XVisualInfo * (*ChooseVisualFuncPtr)(Display *, int, int *);
    ChooseVisualFuncPtr = (XVisualInfo *(*)(Display *, int, int *))
        dlsym(libHandle, "glXChooseVisual");
    if ((dlerror()) != 0)
    {
        return;
    }

    GLXContext(*CreateContextFuncPtr)(Display *, XVisualInfo *, GLXContext, Bool);
    CreateContextFuncPtr = (GLXContext(*)(Display *, XVisualInfo *, GLXContext, Bool))
        dlsym(libHandle, "glXCreateContext");
    if ((dlerror()) != 0)
    {
        return;
    }

    Bool(*MakeCurrentFuncPtr)(Display *, GLXDrawable, GLXContext);
    MakeCurrentFuncPtr = (Bool(*)(Display *, GLXDrawable, GLXContext))
        dlsym(libHandle, "glXMakeCurrent");
    if ((dlerror()) != 0)
    {
        return;
    }

    GLXDrawable(*GetDrawableFuncPtr)();
    GetDrawableFuncPtr = (GLXDrawable(*)())dlsym(libHandle, "glXGetCurrentDrawable");
    if ((dlerror()) != 0)
    {
        return;
    }

    GLXContext(*GetContextFuncPtr)();
    GetContextFuncPtr = (GLXContext(*)())dlsym(libHandle, "glXGetCurrentContext");
    if ((dlerror()) != 0)
    {
        return;
    }

    if (ChooseVisualFuncPtr == 0 || CreateContextFuncPtr == 0 || MakeCurrentFuncPtr == 0 || GetDrawableFuncPtr == 0 || GetContextFuncPtr == 0)
    {
        return;
    }

    int list[30];
    int i = 0;

    list[i++] = GLX_RGBA;
    list[i++] = GLX_DOUBLEBUFFER;
    list[i++] = GLX_RED_SIZE; list[i++] = 8;
    list[i++] = GLX_GREEN_SIZE; list[i++] = 8;
    list[i++] = GLX_BLUE_SIZE; list[i++] = 8;
    list[i++] = GLX_ALPHA_SIZE; list[i++] = 8;
    list[i++] = GLX_DEPTH_SIZE; list[i++] = 24;
    list[i++] = GLX_STENCIL_SIZE; list[i++] = 8;

    list[i++] = None;
    XVisualInfo *vinfo = ChooseVisualFuncPtr(_display, DefaultScreen(_display), list);
    if (vinfo == 0)
    {
        _contextHandle = 0;
        return;
    }

    // Create context that shares display lists and texture objects across contexts
    if (sharedWithContext)
    {
        _contextHandle = CreateContextFuncPtr(_display, vinfo, sharedWithContext, GL_TRUE);
    }
    else
    {
        _contextHandle = CreateContextFuncPtr(_display, vinfo, 0, GL_TRUE);
    }

    if (_contextHandle == 0)
    {
        return;
    }

    // For glX need a window to make the context created above current, creating
    // minimal requirements for an OpenGL window

    Window root = RootWindow(_display, DefaultScreen(_display));
    Colormap cmap = XCreateColormap(_display, root, vinfo->visual, AllocNone);
    XSetWindowAttributes wa;
    unsigned long attribMask;
    attribMask = CWBackPixmap | CWBorderPixel | CWColormap;
    wa.background_pixmap = None;
    wa.border_pixel = 0;
    wa.colormap = cmap;

    // Create an X window with the visual requested above
    _dummyWindow = XCreateWindow(_display, root, 0, 0, 10, 10, 0, vinfo->depth, InputOutput,
        vinfo->visual, attribMask, &wa);
    if (_dummyWindow == 0)
    {
        _contextHandle = 0;
        return;
    }

    //	Save the current context.
    GLXDrawable oldDrawable = GetDrawableFuncPtr();
    GLXContext	oldContext = GetContextFuncPtr();
    bool haveOldContext = (NULL != oldContext);
    bool haveOldDrawable = (None != oldDrawable);

    MakeCurrentFuncPtr(_display, _dummyWindow, _contextHandle);

    if (_display)
    {
        _isValid = true;

        //	Restore the previous context
        if (haveOldContext && haveOldDrawable)
        {
            MakeCurrentFuncPtr(_display, oldDrawable, oldContext);

        }
    }
}

//
// OSX implementation
//
#elif defined(__APPLE__)

GLUtilityContext::GLUtilityContext(const WindowWrapper& /*windowWrapper*/, HardwareContextHandle sharedWithContext)
{
    _isValid = false;

    void* pixelFormat = NSOpenGLChoosePixelFormatWrapper(true, 0, 32, 24, 8, 0, 0, false,
        false, false, false, false);
    if (!pixelFormat)
    {
        return;
    }

    // Create the context, but do not share against other contexts.
    // (Instead, all other contexts will share against this one.)
    //
    _contextHandle = NSOpenGLCreateContextWrapper(pixelFormat, sharedWithContext);
    NSOpenGLReleasePixelFormat(pixelFormat);
    NSOpenGLMakeCurrent(_contextHandle);

    _isValid = true;
}
#endif

// Destroy the startup context.
GLUtilityContext::~GLUtilityContext()
{
    // Only do this portion if the context is valid
    if (_isValid)
    {
#if defined(_WIN32)
        // Release the dummy context.
        wglDeleteContext(_contextHandle);

#elif defined(__linux__)
        glXMakeCurrent(_display, None, NULL);

        // This needs to be done after all the GL object
        // created with this context are destroyed.
        if (_contextHandle != 0)
        {
            glXDestroyContext(_display, _contextHandle);
        }
        if (_dummyWindow != 0)
        {
            XDestroyWindow(_display, _dummyWindow);
        }

#elif defined(__APPLE__)
        // This needs to be done after all the GL object
        // created with this context are destroyed.
        if (_contextHandle != 0)
        {
            NSOpenGLDestroyCurrentContext(&_contextHandle);
        }
#endif
    }
}

int GLUtilityContext::makeCurrent()
{
    if (!_isValid)
    {
        return 0;
    }

    int makeCurrentOk = 0;
#if defined(_WIN32)
    makeCurrentOk = wglMakeCurrent(_dummyWindow.windowWrapper().internalHandle(), _contextHandle);
#elif defined(__linux__)
    makeCurrentOk = glXMakeCurrent(_display, _dummyWindow, _contextHandle);
#elif defined(__APPLE__)
    NSOpenGLMakeCurrent(_contextHandle);
    if (NSOpenGLGetCurrentContextWrapper() == _contextHandle)
    {
        makeCurrentOk = 1;
    }
#else
    ;
#endif
    return makeCurrentOk;
}

//
// Creator
//
GLUtilityContextPtr GLUtilityContext::create(const WindowWrapper& windowWrapper, HardwareContextHandle context)
{
    return std::shared_ptr<GLUtilityContext>(new GLUtilityContext(windowWrapper, context));
}

}
