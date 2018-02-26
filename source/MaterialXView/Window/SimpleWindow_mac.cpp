#if defined(OSMac_)

#include <MaterialXView/Window/SimpleWindow.h>
#include <MaterialXView/Window/CocoaWindowWrappers.h>

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

bool SimpleWindow::create(char* title,
                        unsigned int width, unsigned int height,
                        void* /*applicationShell*/)
{
    void* win = NSOpenGLCreateWindow(width, height, title, true);
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
    NSOpenGLDisposeWindow(hWnd);
}

}
#endif
