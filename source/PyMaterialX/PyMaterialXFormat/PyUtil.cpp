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
    mod.def("readFile", &mx::readFile,
            py::arg("path"),
            R"docstring(
    Read the file with the given `path`, and return its contents.

    :param path: The path of the file to read.
    :type path: FilePath | str
    :return: The contents of the specified file, or an empty string if the file
        could not be read.
)docstring");

    mod.def("getSubdirectories", &mx::getSubdirectories,
            py::arg("rootDirectories"),
            py::arg("searchPath"),
            py::arg("subDirectories"),
            R"docstring(
    Get all subdirectories for a given set of directories and search paths.
)docstring");

    mod.def("loadDocuments", &mx::loadDocuments,
            py::arg("rootPath"),
            py::arg("searchPath"),
            py::arg("skipFiles"),
            py::arg("includeFiles"),
            py::arg("documents"),
            py::arg("documentsPaths"),
            py::arg("readOptions") = (mx::XmlReadOptions*) nullptr,
            py::arg("errors") = (mx::StringVec*) nullptr,
            R"docstring(
    Scan for all documents under the given `rootPath` and return documents
    which can be loaded.
)docstring");

    mod.def("loadLibrary", &mx::loadLibrary,
            py::arg("path"),
            py::arg("doc"),
            py::arg("searchPath") = mx::FileSearchPath(),
            py::arg("readOptions") = (mx::XmlReadOptions*) nullptr,
            R"docstring(
    Load a MaterialX library specified by the given `path` into the given
    document.
)docstring");

    mod.def("loadLibraries", &mx::loadLibraries,
            py::arg("libraryFolders"),
            py::arg("searchPath"),
            py::arg("doc"),
            py::arg("excludeFiles") = mx::StringSet(),
            py::arg("readOptions") = (mx::XmlReadOptions*) nullptr,
            R"docstring(
    Load all MaterialX files within the given library folders into a document,
    using the given search path to locate the folders on the file system.
)docstring");

    mod.def("flattenFilenames", &mx::flattenFilenames,
            py::arg("doc"),
            py::arg("searchPath") = mx::FileSearchPath(),
            py::arg("customResolver") = (mx::StringResolverPtr) nullptr,
            R"docstring(
    Flatten all filenames in the given document, applying string resolvers at
    the scope of each element and removing all fileprefix attributes.

    :param doc: The document to modify.
    :param searchPath: An optional search path for relative to absolute path conversion.
    :param customResolver: An optional custom resolver to apply.
)docstring");

    mod.def("getSourceSearchPath", &mx::getSourceSearchPath,
            py::arg("doc"),
            R"docstring(
    Return a file search path containing the parent folder of each source URI
    in the given document.
)docstring");
}
