//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXCore/Document.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyXmlIo(py::module& mod)
{
    py::class_<mx::XmlReadOptions>(mod, "XmlReadOptions")
        .def(py::init())
        .def_readwrite("readXIncludeFunction", &mx::XmlReadOptions::readXIncludeFunction)
        .def_readwrite("readComments", &mx::XmlReadOptions::readComments)
        .def_readwrite("readNewlines", &mx::XmlReadOptions::readNewlines)
        .def_readwrite("upgradeVersion", &mx::XmlReadOptions::upgradeVersion)        
        .def_readwrite("parentXIncludes", &mx::XmlReadOptions::parentXIncludes)
        .doc() = R"docstring(
    Class providing a set of options for controlling the behavior of XML read
    functions.

    :see: https://materialx.org/docs/api/class_xml_read_options.html
)docstring";

    py::class_<mx::XmlWriteOptions>(mod, "XmlWriteOptions")
        .def(py::init())
        .def_readwrite("writeXIncludeEnable", &mx::XmlWriteOptions::writeXIncludeEnable)
        .def_readwrite("elementPredicate", &mx::XmlWriteOptions::elementPredicate)
        .doc() = R"docstring(
    Class providing a set of options for controlling the behavior of XML write
    functions.

    :see: https://materialx.org/docs/api/class_xml_write_options.html
)docstring";

    mod.def("readFromXmlFile", &mx::readFromXmlFile,
            py::arg("doc"),
            py::arg("filename"),
            py::arg("searchPath") = mx::FileSearchPath(),
            py::arg("readOptions") = (mx::XmlReadOptions*) nullptr,
            R"docstring(
    Read a `Document` as XML from the given filename.

    :param doc: The `Document` into which data is read.
    :type doc: Document
    :param filename: The filename from which data is read.
    :type filename: FilePath | str
    :param searchPath: An optional sequence of file paths that will be applied
        in order when searching for the given file and its includes. If given
        as a `str`, is expected to contain paths separated by the
        `PATH_SEPARATOR` character.
    :type searchPath: FileSearchPath | str
    :param readOptions: An optional `XmlReadOptions` object to affect the
        behavior of the read function.
    :type readOptions: XmlReadOptions
    :raises ExceptionParseError: If the document cannot be parsed.
    :raises ExceptionFileMissing: If the file cannot be opened.
)docstring");

    mod.def("readFromXmlString", &mx::readFromXmlString,
            py::arg("doc"),
            py::arg("string"),
            py::arg("searchPath") = mx::FileSearchPath(),
            py::arg("readOptions") = (mx::XmlReadOptions*) nullptr,
            R"docstring(
    Read a `Document` as XML from the given string.

    :param doc: The `Document` into which data is read.
    :type doc: Document
    :param string: The string from which data is read.
    :type string: str
    :param searchPath: An optional sequence of file paths that will be applied
        in order when searching for the given file and its includes. If given
        as a `str`, is expected to contain paths separated by the
        `PATH_SEPARATOR` character.
    :type searchPath: FileSearchPath | str
    :param readOptions: An optional `XmlReadOptions` object to affect the
        behavior of the read function.
    :type readOptions: XmlReadOptions
    :raises ExceptionParseError: If the document cannot be parsed.
)docstring");

    mod.def("writeToXmlFile", mx::writeToXmlFile,
            py::arg("doc"),
            py::arg("filename"),
            py::arg("writeOptions") = (mx::XmlWriteOptions*) nullptr,
            R"docstring(
    Write a `Document` as XML to the given filename.

    :param doc: The `Document` to write.
    :type doc: Document
    :param filename: The filename to which data is written.
    :type filename: FilePath | str
    :param writeOptions: An optional `XmlWriteOptions` object to affect the
        behavior of the write function.
    :type writeOptions: XmlWriteOptions, optional
)docstring");

    mod.def("writeToXmlString", mx::writeToXmlString,
            py::arg("doc"),
            py::arg("writeOptions") = nullptr,
            R"docstring(
    Write a `Document` as XML to a string and return it.

    :param doc: The `Document` to write.
    :type doc: Document
    :param writeOptions: An optional `XmlWriteOptions` object to affect the
        behavior of the write function.
    :type writeOptions: XmlWriteOptions, optional
    :return: The output string.
)docstring");

    mod.def("prependXInclude", mx::prependXInclude,
            py::arg("doc"),
            py::arg("filename"),
            R"docstring(
    Add an `XInclude` reference to the top of the given `Document`, creating a
    `GenericElement` to hold the reference filename.

    :param doc: The `Document` to be modified.
    :type doc: Document
    :param filename: The filename of the `XInclude` reference to be added.
    :type filename: FilePath
)docstring");

    mod.def("getEnvironmentPath", &mx::getEnvironmentPath,
            py::arg("sep") = mx::PATH_LIST_SEPARATOR,
            R"docstring(
    Return a `FileSearchPath` object from the search path environment variable
    `MATERIALX_SEARCH_PATH`.
)docstring");

    mod.attr("PATH_LIST_SEPARATOR") = mx::PATH_LIST_SEPARATOR;
    mod.attr("MATERIALX_SEARCH_PATH_ENV_VAR") = mx::MATERIALX_SEARCH_PATH_ENV_VAR;

    py::register_exception<mx::ExceptionParseError>(mod, "ExceptionParseError")
        .doc() = R"docstring(
    A type of exception that is raised when a requested document cannot be
    parsed.

    :see: https://materialx.org/docs/api/class_exception_parse_error.html
)docstring";

    py::register_exception<mx::ExceptionFileMissing>(mod, "ExceptionFileMissing")
        .doc() = R"docstring(
    A type of exception that is raised when a requested file cannot be opened.

    :see: https://materialx.org/docs/api/class_exception_file_missing.html
)docstring";
}
