//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#if defined(__APPLE__)

#include <MaterialXRenderHw/SimpleWindow.h>
#include <MaterialXRenderHw/WindowCocoaWrappers.h>

MATERIALX_NAMESPACE_BEGIN

SimpleWindow::SimpleWindow() :
    _width(0),
    _height(0)
{
    // Give a unique identifier to this window.
    static unsigned int windowCount = 1;
    _id = windowCount;
    windowCount++;
}

bool SimpleWindow::initialize(const char* title,
                              unsigned int width, unsigned int height,
                              void* /*applicationShell*/)
{
    void* win = NSUtilCreateWindow(width, height, title, true);
    if (!win)
    {
        return false;
    }
    _windowWrapper = WindowWrapper::create(win);
    return true;
}

SimpleWindow::~SimpleWindow()
{
    void* hWnd = _windowWrapper->externalHandle();
    NSUtilDisposeWindow(hWnd);
}

MATERIALX_NAMESPACE_END

#endif
