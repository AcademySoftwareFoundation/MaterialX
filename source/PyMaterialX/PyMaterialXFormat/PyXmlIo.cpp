//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXCore/Document.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyXmlIo(py::module& mod)
{
    py::class_<mx::XmlReadOptions, mx::CopyOptions>(mod, "XmlReadOptions")
        .def(py::init())
        .def_readwrite("readXIncludeFunction", &mx::XmlReadOptions::readXIncludeFunction)
        .def_readwrite("parentFilenames", &mx::XmlReadOptions::parentFilenames);

    py::class_<mx::XmlWriteOptions>(mod, "XmlWriteOptions")
        .def(py::init())
        .def_readwrite("writeXIncludeEnable", &mx::XmlWriteOptions::writeXIncludeEnable)
        .def_readwrite("elementPredicate", &mx::XmlWriteOptions::elementPredicate);

    mod.def("readFromXmlFileBase", &mx::readFromXmlFile,
        py::arg("doc"), py::arg("filename"), py::arg("searchPath") = mx::EMPTY_STRING, py::arg("readOptions") = (mx::XmlReadOptions*) nullptr);
    mod.def("readFromXmlString", &mx::readFromXmlString,
        py::arg("doc"), py::arg("str"), py::arg("readOptions") = (mx::XmlReadOptions*) nullptr);
    mod.def("writeToXmlFile", mx::writeToXmlFile,
        py::arg("doc"), py::arg("filename"), py::arg("writeOptions") = (mx::XmlWriteOptions*) nullptr);
    mod.def("writeToXmlString", mx::writeToXmlString,
        py::arg("doc"), py::arg("writeOptions") = nullptr);
    mod.def("prependXInclude", mx::prependXInclude);

    py::register_exception<mx::ExceptionParseError>(mod, "ExceptionParseError");
    py::register_exception<mx::ExceptionFileMissing>(mod, "ExceptionFileMissing");
}
