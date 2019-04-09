//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/HardwarePlatform.h>

#if defined(OSMac_)

#include <MaterialXRenderHw/Window/SimpleWindow.h>
#include <MaterialXRenderHw/Window/WindowCocoaWrappers.h>

namespace MaterialX
{
SimpleWindow::SimpleWindow()
{
    clearInternalState();

    // Give a unique identifier to this window.
    static unsigned int windowCount = 1;
    _id = windowCount;
    windowCount++;
}

bool SimpleWindow::initialize(char* title,
                              unsigned int width, unsigned int height,
                              void* /*applicationShell*/)
{
    void* win = NSUtilCreateWindow(width, height, title, true);
    if (!win)
    {
        return false;
    }
    _windowWrapper = WindowWrapper(win);
    return true;
}

SimpleWindow::~SimpleWindow()
{
    void* hWnd = _windowWrapper.externalHandle();
    NSUtilDisposeWindow(hWnd);
}
}
#endif
