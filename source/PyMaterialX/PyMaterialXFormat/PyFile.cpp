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
    py::enum_<mx::FilePath::Type>(mod, "Type",
                                  PYMATERIALX_DOCSTRING(R"docstring(
    Enumeration of `FilePath` types.

    :see: https://materialx.org/docs/api/class_file_path.html#pub-types
)docstring"))

        .value("TypeRelative", mx::FilePath::Type::TypeRelative,
               PYMATERIALX_DOCSTRING(R"docstring(
    Type indicating a relative path, e.g. `'a/b'`.

    This is the default file path type.
)docstring"))

        .value("TypeAbsolute", mx::FilePath::Type::TypeAbsolute,
               PYMATERIALX_DOCSTRING(R"docstring(
    Type indicating an absolute path, e.g. `'/a/b'`.
)docstring"))

        .value("TypeNetwork", mx::FilePath::Type::TypeNetwork,
               PYMATERIALX_DOCSTRING(R"docstring(
    Type indicating a network path on Windows, e.g. `'\\\\a\\b'`.
)docstring"))

        .export_values();
    // Quirk: The `Type` enum is exposed at the module level, not as part of
    // the `FilePath` class

    py::enum_<mx::FilePath::Format>(mod, "Format",
                                    PYMATERIALX_DOCSTRING(R"docstring(
    Enumeration of `FilePath` formats.

    :see: https://materialx.org/docs/api/class_file_path.html#pub-types
)docstring"))

        .value("FormatWindows", mx::FilePath::Format::FormatWindows,
               PYMATERIALX_DOCSTRING(R"docstring(
    Format indicating a Windows environment.
)docstring"))

        .value("FormatPosix", mx::FilePath::Format::FormatPosix,
               PYMATERIALX_DOCSTRING(R"docstring(
    Format indicating a Linux or Mac environment.
)docstring"))

        .value("FormatNative", mx::FilePath::Format::FormatNative,
               PYMATERIALX_DOCSTRING(R"docstring(
    Format indicating the format used on the system for which the library was
    built.
)docstring"))

        .export_values();
    // Quirk: The `Format` enum is exposed at the module level, not as part of
    // the `FilePath` class

    py::class_<mx::FilePath>(mod, "FilePath")

        .def(py::init<>(),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class as an empty relative path.
)docstring"))

        .def(py::init<const std::string&>(),
             py::arg("path"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class based on the given `path`.
)docstring"))

        .def(py::self == py::self,
             py::arg("other"))

        .def(py::self != py::self,
             py::arg("other"))

        .def(py::self / py::self,
             py::arg("rhs"))

        .def("asString", &mx::FilePath::asString,
             py::arg_v("format",
                       mx::FilePath::Format::FormatNative,
                       "mx.Format.FormatNative"),
                       // Quirk: The `Format` enum is exposed at the module
                       // level, not as part of the `FilePath` class
             PYMATERIALX_DOCSTRING(R"docstring(
    Return this path as a string with the given format.
)docstring"))

        .def("isEmpty", &mx::FilePath::isEmpty,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this path is empty.
)docstring"))

        .def("isAbsolute", &mx::FilePath::isAbsolute,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this path is absolute.
)docstring"))

        .def("getBaseName", &mx::FilePath::getBaseName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the base name of this path, with leading directory information
    removed.
)docstring"))

        .def("getParentPath", &mx::FilePath::getParentPath,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the parent directory of this path, if any.

    If no parent directory is present, then the empty path is returned.
)docstring"))

        .def("getExtension", &mx::FilePath::getExtension,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the file extension of this path.
)docstring"))

        .def("addExtension", &mx::FilePath::addExtension,
             py::arg("ext"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a file extension to this path.
)docstring"))

        .def("removeExtension", &mx::FilePath::removeExtension,
             PYMATERIALX_DOCSTRING(R"docstring(
    Remove the file extension, if any, from this path.
)docstring"))

        .def("size", &mx::FilePath::size,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the number of parts in this path.
)docstring"))

        .def("getNormalized", &mx::FilePath::getNormalized,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a normalized version of this path, collapsing current path and
    parent path references.

    >>> mx.FilePath('a/./b').getNormalized().asString()
    'a/b'
    >>> mx.FilePath('c/../d/../a/b').getNormalized().asString()
    'a/b'
)docstring"))

        .def("exists", &mx::FilePath::exists,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this path exists on the file system.
)docstring"))

        .def("isDirectory", &mx::FilePath::isDirectory,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this path is a directory on the file system.
)docstring"))

        .def("getFilesInDirectory", &mx::FilePath::getFilesInDirectory,
             py::arg("extension"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all files in this directory with the given extension.
)docstring"))

        .def("getSubDirectories", &mx::FilePath::getSubDirectories,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a list of all directories at or beneath this path.
)docstring"))

        .def("createDirectory", &mx::FilePath::createDirectory,
             PYMATERIALX_DOCSTRING(R"docstring(
    Create a directory on the file system at this path.
)docstring"))

        .def_static("getCurrentPath", &mx::FilePath::getCurrentPath,
                    PYMATERIALX_DOCSTRING(R"docstring(
    Return the current working directory of the file system.
)docstring"))

        .def_static("getModulePath", &mx::FilePath::getModulePath,
                    PYMATERIALX_DOCSTRING(R"docstring(
    Return the directory containing the executable module.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a generic file path, supporting both syntactic and file
    system operations.

    :see: https://materialx.org/docs/api/class_file_path.html
)docstring");

    py::class_<mx::FileSearchPath>(mod, "FileSearchPath")

        .def(py::init<>(),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class to represent a default search path.
)docstring"))

        .def(py::init<const std::string&, const std::string&>(),
             py::arg("searchPath"),
             py::arg_v("sep",
                       mx::PATH_LIST_SEPARATOR,
                       "mx.PATH_LIST_SEPARATOR"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class to represent the given `searchPath`,
    using the given `sep` string as a path list separator.
)docstring"))

        .def("asString", &mx::FileSearchPath::asString,
             py::arg_v("sep",
                       mx::PATH_LIST_SEPARATOR,
                       "mx.PATH_LIST_SEPARATOR"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return this search path as a string, joining its parts using the given
    `sep` string.
)docstring"))

        .def("append", static_cast<void (mx::FileSearchPath::*)(const mx::FilePath&)>(&mx::FileSearchPath::append),
             py::arg("path"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Append the given `path` to the sequence.
)docstring"))

        .def("append", static_cast<void (mx::FileSearchPath::*)(const mx::FileSearchPath&)>(&mx::FileSearchPath::append),
             py::arg("searchPath"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Append the given `searchPath` to the sequence.
)docstring"))

        .def("prepend", &mx::FileSearchPath::prepend,
             py::arg("path"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Prepend the given `path` to the sequence.
)docstring"))

        .def("clear", &mx::FileSearchPath::clear,
             PYMATERIALX_DOCSTRING(R"docstring(
    Clear all paths from the sequence.
)docstring"))

        .def("size", &mx::FileSearchPath::size,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the number of paths in the sequence.
)docstring"))

        .def("isEmpty", &mx::FileSearchPath::isEmpty,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return true if the search path is empty.
)docstring"))

        .def("find", &mx::FileSearchPath::find,
             py::arg("filename"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Given an input `filename`, iterate through each path in this sequence,
    returning the first combined path found on the file system.

    On success, the combined path is returned; otherwise the original
    filename is returned unmodified.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a sequence of file paths, which may be queried to find
    the first instance of a given filename on the file system.

    :see: https://materialx.org/docs/api/class_file_search_path.html
)docstring");

    py::implicitly_convertible<std::string, mx::FilePath>();
    py::implicitly_convertible<std::string, mx::FileSearchPath>();

    mod.attr("PATH_LIST_SEPARATOR") = mx::PATH_LIST_SEPARATOR;
    mod.attr("MATERIALX_SEARCH_PATH_ENV_VAR") = mx::MATERIALX_SEARCH_PATH_ENV_VAR;
}
