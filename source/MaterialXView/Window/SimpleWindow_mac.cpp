#include <MaterialXView/Window/HardwarePlatform.h>

#if defined(OSMac_)

#include <MaterialXView/Window/SimpleWindow.h>
#include <MaterialXView/Window/WindowCocoaWrappers.h>

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
