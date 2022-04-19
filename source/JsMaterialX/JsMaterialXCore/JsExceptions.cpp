//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <emscripten/bind.h>

#include <string>

namespace ems = emscripten;

namespace jsexceptions
{
std::string getExceptionMessage(int exceptionPtr)
{
    return std::string(reinterpret_cast<std::exception *>(exceptionPtr)->what());
}
} // namespace jsexceptions

EMSCRIPTEN_BINDINGS(exceptions)
{
    ems::function("getExceptionMessage", &jsexceptions::getExceptionMessage);
}
