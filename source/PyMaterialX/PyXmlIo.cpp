//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXCore/Document.h>

#include <PyBind11/operators.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyXmlIo(py::module& mod)
{
    py::class_<mx::XmlReadOptions>(mod, "XmlReadOptions")
        .def(py::init())
        .def("setReadXincludes", &mx::XmlReadOptions::setReadXincludes)
        .def("getReadXincludes", &mx::XmlReadOptions::getReadXincludes)
        .def("setSkipDuplicates", &mx::XmlReadOptions::setSkipDuplicates)
        .def("getSkipDuplicates", &mx::XmlReadOptions::getSkipDuplicates);

    mod.def("readFromXmlFileBase", &mx::readFromXmlFile,
        py::arg("doc"), py::arg("filename"), py::arg("searchPath") = mx::EMPTY_STRING, py::arg("readOptions") = (const mx::XmlReadOptions*)nullptr);
    mod.def("readFromXmlString", &mx::readFromXmlString,
        py::arg("doc"), py::arg("str"), py::arg("readOptions") = (const mx::XmlReadOptions*)nullptr);
    mod.def("writeToXmlFile", mx::writeToXmlFile,
        py::arg("doc"), py::arg("filename"), py::arg("writeXIncludes") = true, py::arg("predicate") = mx::ElementPredicate());
    mod.def("writeToXmlString", mx::writeToXmlString,
        py::arg("doc"), py::arg("writeXIncludes") = true, py::arg("predicate") = mx::ElementPredicate());
    mod.def("prependXInclude", mx::prependXInclude);

    py::register_exception<mx::ExceptionParseError>(mod, "ExceptionParseError");
    py::register_exception<mx::ExceptionFileMissing>(mod, "ExceptionFileMissing");
}
