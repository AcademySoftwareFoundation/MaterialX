//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRenderHw/Window/SimpleWindow.h>

namespace MaterialX
{

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

}
