//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXFormat/File.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyFile(py::module& mod)
{
    py::enum_<mx::FilePath::Type>(mod, "Type")
        .value("TypeRelative", mx::FilePath::Type::TypeRelative)
        .value("TypeAbsolute", mx::FilePath::Type::TypeAbsolute)
        .value("TypeNetwork", mx::FilePath::Type::TypeNetwork)
        .export_values();

    py::enum_<mx::FilePath::Format>(mod, "Format")
        .value("FormatWindows", mx::FilePath::Format::FormatWindows)
        .value("FormatPosix", mx::FilePath::Format::FormatPosix)
        .value("FormatNative", mx::FilePath::Format::FormatNative)
        .export_values();

    py::class_<mx::FilePath>(mod, "FilePath")
        .def_static("getCurrentPath", &mx::FilePath::getCurrentPath)
        .def(py::init<>())
        .def(py::init<const std::string&>())
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def(py::self / py::self)
        .def("assign", &mx::FilePath::assign)
        .def("asString", &mx::FilePath::asString)
        .def("isEmpty", &mx::FilePath::isEmpty)
        .def("isAbsolute", &mx::FilePath::isAbsolute)
        .def("getBaseName", &mx::FilePath::getBaseName)
        .def("exists", &mx::FilePath::exists);

    mod.attr("PATH_LIST_SEPARATOR") = mx::PATH_LIST_SEPARATOR;
    mod.attr("MATERIALX_SEARCH_PATH_ENV_VAR") = mx::MATERIALX_SEARCH_PATH_ENV_VAR;
}
