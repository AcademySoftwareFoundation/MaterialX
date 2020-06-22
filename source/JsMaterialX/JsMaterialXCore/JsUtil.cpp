#include "../helpers.h"
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Util.h>

#include <MaterialXFormat/File.h>
#include <iostream>
#include <vector>
#include <iterator>
#include <map>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

extern "C"
{
    EMSCRIPTEN_BINDINGS(my_module)
    {
        ems::function("getVersionString", &mx::getVersionString);
        ems::function("createValidName", &mx::createValidName); // arg0 === {std::string}, arg1 === {unicode representing character}
        ems::function("makeVersionString", &mx::makeVersionString);
        ems::function("isValidName", &mx::isValidName);
        ems::function("incrementName", &mx::incrementName);

        // The following function throw: Cannot call {function name} due to unbound types: XXXXX
        ems::function("getVersionIntegers", ems::optional_override([]() {
                     std::tuple<int, int, int> version = mx::getVersionIntegers();
                     return arrayToVec((int *)&version, 3);
                 }));

        ems::function("splitString", ems::optional_override([](std::string str, std::string sep) {
                     const std::string &str1 = str;
                     const std::string &sep2 = sep;
                     return mx::splitString(str1, sep2);
                 }));

        ems::function("replaceSubstrings", ems::optional_override([](std::string str, ems::val newValue) {
                     mx::StringMap separatorMapper;
                     ems::val keys = ems::val::global("Object").call<ems::val>("keys", newValue);
                     int length = keys["length"].as<int>();
                     for (int i = 0; i < length; ++i)
                     {
                         std::string key = keys[i].as<std::string>().c_str();
                         std::string value = newValue[key].as<std::string>();
                         separatorMapper[key] = value;
                     }
                     return mx::replaceSubstrings(str, separatorMapper);
                 }));

        ems::function("prettyPrint", &mx::prettyPrint);
    }
}