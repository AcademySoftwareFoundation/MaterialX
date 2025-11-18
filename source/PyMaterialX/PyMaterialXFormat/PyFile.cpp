//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
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

    py::class_<mx::FilePath>(mod, "FilePath", "A generic file path, supporting both syntactic and file system operations.")
        .def(py::init<>())
        .def(py::init<const std::string&>())
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def(py::self / py::self)
        .def("asString", &mx::FilePath::asString, py::arg("format") = mx::FilePath::Format::FormatNative, "Return this path as a standard string with the given format.")
        .def("isEmpty", &mx::FilePath::isEmpty, "Return true if the given path is empty.")
        .def("isAbsolute", &mx::FilePath::isAbsolute, "Return true if the given path is absolute.")
        .def("getBaseName", &mx::FilePath::getBaseName, "Return the base name of the given path, with leading directory information removed.")
        .def("getParentPath", &mx::FilePath::getParentPath, "Return the parent directory of the given path, if any.\n\nIf no parent directory is present, then the empty path is returned.")
        .def("getExtension", &mx::FilePath::getExtension, "Return the file extension of the given path.")
        .def("addExtension", &mx::FilePath::addExtension, "Add a file extension to the given path.")
        .def("removeExtension", &mx::FilePath::removeExtension, "Remove the file extension, if any, from the given path.")
        .def("size", &mx::FilePath::size, "Return the number of strings in the path.")
        .def("getNormalized", &mx::FilePath::getNormalized, "Return a normalized version of the given path, collapsing current path and parent path references so that 'a/.\n\n/b' and 'c/../d/../a/b' become 'a/b'.")        
        .def("exists", &mx::FilePath::exists, "Return true if the given path exists on the file system.")
        .def("isDirectory", &mx::FilePath::isDirectory, "Return true if the given path is a directory on the file system.")
        .def("getFilesInDirectory", &mx::FilePath::getFilesInDirectory, "Return a vector of all files in the given directory with the given extension.\n\nIf extension is empty all files are returned.")
        .def("getSubDirectories", &mx::FilePath::getSubDirectories, "Return a vector of all directories at or beneath the given path.")
        .def("createDirectory", &mx::FilePath::createDirectory, "Create a directory on the file system at the given path.")
        .def_static("getCurrentPath", &mx::FilePath::getCurrentPath, "Return the current working directory of the file system.")
        .def_static("getModulePath", &mx::FilePath::getModulePath, "Return the directory containing the executable module.");

    py::class_<mx::FileSearchPath>(mod, "FileSearchPath", "A sequence of file paths, which may be queried to find the first instance of a given filename on the file system.")
        .def(py::init<>())
        .def(py::init<const std::string&, const std::string&>(),
             py::arg("searchPath"), py::arg("sep") = mx::PATH_LIST_SEPARATOR)
        .def("asString", &mx::FileSearchPath::asString, py::arg("sep") = mx::PATH_LIST_SEPARATOR, "Convert this sequence to a string using the given separator.")
        .def("append", static_cast<void (mx::FileSearchPath::*)(const mx::FilePath&)>(&mx::FileSearchPath::append), "Append the given search path to the sequence.")
        .def("append", static_cast<void (mx::FileSearchPath::*)(const mx::FileSearchPath&)>(&mx::FileSearchPath::append), "Append the given search path to the sequence.")
        .def("prepend", &mx::FileSearchPath::prepend, "Prepend the given path to the sequence.")
        .def("clear", &mx::FileSearchPath::clear, "Clear all paths from the sequence.")
        .def("size", &mx::FileSearchPath::size, "Return the number of paths in the sequence.")
        .def("isEmpty", &mx::FileSearchPath::isEmpty, "Return true if the search path is empty.")
        .def("find", &mx::FileSearchPath::find, "Given an input filename, iterate through each path in this sequence, returning the first combined path found on the file system.\n\nOn success, the combined path is returned; otherwise the original filename is returned unmodified.");

    py::implicitly_convertible<std::string, mx::FilePath>();
    py::implicitly_convertible<std::string, mx::FileSearchPath>();

    mod.attr("PATH_LIST_SEPARATOR") = mx::PATH_LIST_SEPARATOR;
    mod.attr("MATERIALX_SEARCH_PATH_ENV_VAR") = mx::MATERIALX_SEARCH_PATH_ENV_VAR;
}
