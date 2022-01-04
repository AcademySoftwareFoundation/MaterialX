//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXFormat/Environ.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(environ)
{
    ems::function("getEnviron", &mx::getEnviron);
    ems::function("setEnviron", &mx::setEnviron);
    ems::function("removeEnviron", &mx::removeEnviron);
}
