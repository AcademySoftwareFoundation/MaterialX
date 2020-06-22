#include <string>
#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;

namespace exceptions
{
std::string getExceptionMessage(int exceptionPtr)
{
    return std::string(reinterpret_cast<std::exception *>(exceptionPtr)->what());
}
} // namespace exceptions

extern "C"
{
    EMSCRIPTEN_BINDINGS(exceptions)
    {
        ems::function("getExceptionMessage", &exceptions::getExceptionMessage);
    }
}