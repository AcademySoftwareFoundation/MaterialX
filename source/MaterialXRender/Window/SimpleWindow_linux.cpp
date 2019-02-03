#include <MaterialXRender/Window/HardwarePlatform.h>

#if defined(OSLinux_)

#include <MaterialXRender/Window/SimpleWindow.h>

#include <X11/StringDefs.h>
#include <X11/Shell.h> // for applicationShellWidgetClass
#include <X11/Xlib.h> // for XEvent definition
#include <X11/Intrinsic.h> // for XtCallbackProc definition

namespace MaterialX
{

SimpleWindow::SimpleWindow()
{
    clearInternalState();

    // Give a unique ID to this window.
    //
    static unsigned int windowCount = 1;
    _id = windowCount;
    windowCount++;
}

bool SimpleWindow::initialize(char* title,
                              unsigned int width, unsigned int height,
                              void *applicationShell)
{
    int n = 0;

    XtAppContext appContext;
    Widget shell;
    static Widget batchShell;
    if (!applicationShell)
    {
        static bool initializedXServer = false;
        // Connect to the X Server
        if (!initializedXServer)
        {
            batchShell = XtOpenApplication(&appContext, "__mx_dummy__app__",
                0, 0, &n, 0, 0,
                applicationShellWidgetClass, 0, 0);
            initializedXServer = true;
        }
        shell = batchShell;
    }
    else
    {
        // Reuse existing application shell;
        shell = (Widget)applicationShell;
    }

    if (!shell)
    {
        _id = 0;
        return false;;
    }

    Arg args[6];
    n = 0;
    XtSetArg(args[n], XtNx, 0); n++;
    XtSetArg(args[n], XtNy, 0); n++;
    XtSetArg(args[n], XtNwidth, width); n++;
    XtSetArg(args[n], XtNheight, height); n++;
    Widget widget = XtCreatePopupShell(title, topLevelShellWidgetClass, shell, args, n);
    if (!widget)
    {
        _id = 0;
        return false;
    }

    XtRealizeWidget(widget);
    _windowWrapper = WindowWrapper(widget, XtWindow(widget), XtDisplay(widget));

    return true;
}

SimpleWindow::~SimpleWindow()
{
    Widget widget = _windowWrapper.externalHandle();
    if (widget)
    {
        // Unrealize the widget first to avoid X calls to it
        XtUnrealizeWidget(widget);
        XtDestroyWidget(widget);
        widget = nullptr;
    }
}

}
#endif
