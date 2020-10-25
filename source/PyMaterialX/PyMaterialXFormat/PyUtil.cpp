//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Node.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyUtil(py::module& mod)
{
    mod.def("readFile", &mx::readFile);
    mod.def("getSubdirectories", &mx::getSubdirectories);
    mod.def("loadDocuments", &mx::loadDocuments);
    mod.def("loadLibrary", &mx::loadLibrary,
        py::arg("file"), py::arg("doc"), py::arg("searchPath") = mx::FileSearchPath(), py::arg("readOptions") = (mx::XmlReadOptions*) nullptr);
    mod.def("loadLibraries", &mx::loadLibraries,
        py::arg("libraryFolders"), py::arg("searchPath"), py::arg("doc"), py::arg("excludeFiles") = mx::StringSet(), py::arg("readOptions") = (mx::XmlReadOptions*) nullptr);
    mod.def("resolveFileNames", &mx::resolveFileNames,
        py::arg("doc"), py::arg("searchPath") = mx::FileSearchPath(), py::arg("customResolver") = (mx::StringResolverPtr) nullptr);
}
