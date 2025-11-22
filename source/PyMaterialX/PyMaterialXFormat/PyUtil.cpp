//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Node.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyUtil(py::module& mod)
{
    mod.def("readFile", &mx::readFile, "Read the given file and return a string containing its contents; if the read is not successful, then the empty string is returned.");
    mod.def("getSubdirectories", &mx::getSubdirectories, "Get all subdirectories for a given set of directories and search paths.");
    mod.def("loadDocuments", &mx::loadDocuments, py::arg("rootPath"), py::arg("searchPath"), py::arg("skipFiles"), py::arg("includeFiles"), py::arg("documents"), py::arg("documentsPaths"), py::arg("readOptions") = (mx::XmlReadOptions*) nullptr, py::arg("errors") = (mx::StringVec*) nullptr, "Scans for all documents under a root path and returns documents which can be loaded.");
    mod.def("loadLibrary", &mx::loadLibrary, py::arg("file"), py::arg("doc"), py::arg("searchPath") = mx::FileSearchPath(), py::arg("readOptions") = (mx::XmlReadOptions*) nullptr, "Load a given MaterialX library into a document.");
    mod.def("loadLibraries", &mx::loadLibraries, py::arg("libraryFolders"), py::arg("searchPath"), py::arg("doc"), py::arg("excludeFiles") = mx::StringSet(), py::arg("readOptions") = (mx::XmlReadOptions*) nullptr, "Load all MaterialX files within the given library folders into a document, using the given search path to locate the folders on the file system.");
    mod.def("flattenFilenames", &mx::flattenFilenames, py::arg("doc"), py::arg("searchPath") = mx::FileSearchPath(), py::arg("customResolver") = (mx::StringResolverPtr) nullptr, "Flatten all filenames in the given document, applying string resolvers at the scope of each element and removing all fileprefix attributes.\n\nArgs:\n    doc: The document to modify.\n    searchPath: An optional search path for relative to absolute path conversion.\n    customResolver: An optional custom resolver to apply.");
    mod.def("getSourceSearchPath", &mx::getSourceSearchPath, "Return a file search path containing the parent folder of each source URI in the given document.");
}
