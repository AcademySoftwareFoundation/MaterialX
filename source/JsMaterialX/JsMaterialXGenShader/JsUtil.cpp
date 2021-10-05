//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <JsMaterialX/Helpers.h>
#include <MaterialXCore/Element.h>
#include <MaterialXGenShader/Util.h>

#include <string>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(Util)
{
    BIND_FUNC("isTransparentSurface", mx::isTransparentSurface, 1, 2, mx::ElementPtr, const std::string&);
}
