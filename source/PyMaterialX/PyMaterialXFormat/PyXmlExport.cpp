//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXFormat/XmlExport.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyXmlExport(py::module& mod)
{
    py::class_<mx::XmlExportOptions>(mod, "XmlExportOptions")
        .def(py::init())
        .def_readwrite("mergeLooks", &mx::XmlExportOptions::mergeLooks)
        .def_readwrite("lookGroupToMerge", &mx::XmlExportOptions::lookGroupToMerge)
        .def_readwrite("flattenFilenames", &mx::XmlExportOptions::flattenFilenames)
        .def_readwrite("resolvedTexturePath", &mx::XmlExportOptions::resolvedTexturePath)
        .def_readwrite("stringResolver", &mx::XmlExportOptions::stringResolver);

    mod.def("exportToXmlFile", mx::exportToXmlFile,
        py::arg("doc"), py::arg("filename"), py::arg("exportOptions") = (mx::XmlExportOptions*) nullptr);
    mod.def("exportToXmlString", mx::exportToXmlString,
        py::arg("doc"), py::arg("exportOptions") = nullptr);
}
