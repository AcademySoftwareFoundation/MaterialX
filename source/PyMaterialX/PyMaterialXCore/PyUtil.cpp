//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Node.h>
#include <MaterialXCore/Util.h>

#include <MaterialXFormat/File.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyUtil(py::module& mod)
{
    mod.def("getVersionString", &mx::getVersionString, "Return the version of the MaterialX library as a string.");
    mod.def("getVersionIntegers", &mx::getVersionIntegers, "Return the major, minor, and build versions of the MaterialX library as an integer tuple.");
    mod.def("createValidName", &mx::createValidName, py::arg("name"), py::arg("replaceChar") = '_', "Create a valid MaterialX name from the given string.");
    mod.def("isValidName", &mx::isValidName, "Return true if the given string is a valid MaterialX name.");
    mod.def("incrementName", &mx::incrementName, "Increment the numeric suffix of a name.");
    mod.def("splitString", &mx::splitString, "Split a string into a vector of substrings using the given set of separator characters.");
    mod.def("joinStrings", &mx::joinStrings, "Join a vector of substrings into a single string, placing the given separator between each substring.");
    mod.def("replaceSubstrings", &mx::replaceSubstrings, "Apply the given substring substitutions to the input string.");
    mod.def("stringStartsWith", &mx::stringStartsWith, "Return true if the given string starts with the given prefix.");
    mod.def("stringEndsWith", &mx::stringEndsWith, "Return true if the given string ends with the given suffix.");
    mod.def("splitNamePath", &mx::splitNamePath, "Split a name path into string vector.");
    mod.def("createNamePath", &mx::createNamePath, "Create a name path from a string vector.");
    mod.def("parentNamePath", &mx::parentNamePath, "Given a name path, return the parent name path.");
}
