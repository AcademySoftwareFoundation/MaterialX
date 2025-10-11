//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <emscripten/bind.h>

#include <exception>
#include <string>

namespace ems = emscripten;

namespace jsexceptions
{
std::string getExceptionMessage(ems::val exceptionLike)
{
    try
    {
        const std::string type = exceptionLike.typeOf().as<std::string>();
        if (type == "number")
        {
            // Legacy: numeric pointer to std::exception
            int exceptionPtr = exceptionLike.as<int>();
            return std::string(reinterpret_cast<std::exception *>(exceptionPtr)->what());
        }
        if (type == "string")
        {
            return exceptionLike.as<std::string>();
        }
        if (type == "object")
        {
            // Try typical Error-like object with a message property
            bool hasMessage = false;
            try { hasMessage = exceptionLike.call<bool>("hasOwnProperty", std::string("message")); } catch (...) {}
            if (hasMessage)
            {
                return exceptionLike["message"].as<std::string>();
            }
            // Fallback to toString()
            try { return exceptionLike.call<std::string>("toString"); } catch (...) {}
        }
    }
    catch (...)
    {
        // Fall through to default
    }
    return std::string("Unknown exception");
}
} // namespace jsexceptions

EMSCRIPTEN_BINDINGS(exceptions)
{
    ems::function("getExceptionMessage", &jsexceptions::getExceptionMessage);
}
