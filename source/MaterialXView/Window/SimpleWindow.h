#ifndef MATERIALX_SIMPLEWINDOW_H
#define MATERIALX_SIMPLEWINDOW_H

#include <MaterialXView/Window/HardwarePlatform.h>
#include <MaterialXView/Window/WindowWrapper.h>

#include <memory>

namespace MaterialX
{

// SimpleWindow shared pointer
using SimpleWindowPtr = std::shared_ptr<class SimpleWindow>;

///
/// @class SimpleWindow
/// A platform independent window class. Plaform specific resources
/// are encapsulated using a WindowWrapper instance.
///
class SimpleWindow
{
  public:
    /// Static instance create function
    static SimpleWindowPtr create() { return std::make_shared<SimpleWindow>(); }

    /// Default constructor
    SimpleWindow();

    /// Default destructor
    virtual ~SimpleWindow();

    /// Window initialization
    bool initialize(char* title, unsigned int width, unsigned int height,
        void *applicationShell);

    /// Return windowing information for the window
    const WindowWrapper& windowWrapper()
    {
        return _windowWrapper;
    }

    /// Return width of window
    unsigned int width() const
    {
        return _width;
    }

    /// Return height of window
    unsigned int height() const
    {
        return _height;
    }

    /// Check for validity
    bool isValid() const
    {
        return _windowWrapper.isValid();
    }

  protected:
    /// Clear internal state information
    void clearInternalState()
    {
        _width = _height = 0;
        _id = 0;
    }

    /// Wrapper for platform specific window resources
    WindowWrapper _windowWrapper;
    
    /// Width of the window
    unsigned int _width;
    /// Height of the window
    unsigned int _height;

    /// Unique window identifier generated dynamically at creation time.
    unsigned int _id;

#if defined(OSWin_)
    /// Window class name for window generated at creation time.
    char _windowClassName[128];
#endif

};

#if defined(OSUnsupported_)

//
// Stubs for unsupported OS
//
SimpleWindow::SimpleWindow() :
    _id(0)
{
}

SimpleWindow::~SimpleWindow()
{
}

bool SimpleWindow::initialize(char* /*title*/,
    unsigned int /*width*/,
    unsigned int /*height*/,
    void* /*applicationShell*/)
{
    return false;
}

#endif

} // namespace MaterialX

#endif
