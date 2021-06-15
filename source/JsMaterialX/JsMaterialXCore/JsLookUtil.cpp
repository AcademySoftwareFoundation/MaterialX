//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include "../VectorHelper.h"

#include <MaterialXCore/LookUtil.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(lookUtil)
{
    ems::function("getActiveLooks", &mx::getActiveLooks);
    ems::function("appendLookGroup", &mx::appendLookGroup);
    ems::function("appendLook", &mx::appendLook);
    ems::function("combineLooks", &mx::combineLooks);
}

