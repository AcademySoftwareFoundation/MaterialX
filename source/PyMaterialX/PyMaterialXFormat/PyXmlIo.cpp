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

        .def(py::init(),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class using default options.
)docstring"))

        .def_readwrite("readXIncludeFunction", &mx::XmlReadOptions::readXIncludeFunction,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`XmlReadFunction`)
    If provided, this function will be invoked when an XInclude reference
    needs to be read into a document.

    Defaults to `readFromXmlFile()`.
)docstring"))

        .def_readwrite("readComments", &mx::XmlReadOptions::readComments,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`bool`)
    If true, then XML comments will be read into documents as comment elements.

    Defaults to `False`.
)docstring"))

        .def_readwrite("readNewlines", &mx::XmlReadOptions::readNewlines,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`bool`)
    If true, then XML newlines will be read into documents as newline elements.

    Defaults to `False`.
)docstring"))

        .def_readwrite("upgradeVersion", &mx::XmlReadOptions::upgradeVersion,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`bool`)
    If `True`, then documents from earlier versions of MaterialX will be
    upgraded to the current version.

    Defaults to `True`.

    :see: `PyMaterialXCore.Document.upgradeVersion()`
    :see: `PyMaterialXCore.getVersionString()`
)docstring"))

        .def_readwrite("parentXIncludes", &mx::XmlReadOptions::parentXIncludes,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`List[str]`)
    The list of parent XIncludes at the scope of the current document.

    Defaults to an empty list.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class providing a set of options for controlling the behavior of XML read
    functions.

    :see: https://materialx.org/docs/api/class_xml_read_options.html
)docstring");

    py::class_<mx::XmlWriteOptions>(mod, "XmlWriteOptions")

        .def(py::init(),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class using default options.
)docstring"))

        .def_readwrite("writeXIncludeEnable", &mx::XmlWriteOptions::writeXIncludeEnable,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`bool`)
    If `True`, elements with source file markings will be written as XIncludes
    rather than explicit data.

    Defaults to `True`.
)docstring"))

        .def_readwrite("elementPredicate", &mx::XmlWriteOptions::elementPredicate,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`ElementPredicate`)
    If provided, this function will be used to exclude specific elements (those
    returning `False`) from the write operation.

    Defaults to `None`.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class providing a set of options for controlling the behavior of XML write
    functions.

    :see: https://materialx.org/docs/api/class_xml_write_options.html
)docstring");

    mod.def("readFromXmlFile", &mx::readFromXmlFile,
            py::arg("doc"),
            py::arg("filename"),
            py::arg_v("searchPath",
                      mx::FileSearchPath(),
                      "mx.FileSearchPath()"),
            py::arg("readOptions") = (mx::XmlReadOptions*) nullptr,
            PYMATERIALX_DOCSTRING(R"docstring(
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
)docstring"));

    mod.def("readFromXmlString", &mx::readFromXmlString,
            py::arg("doc"),
            py::arg("string"),
            py::arg_v("searchPath",
                      mx::FileSearchPath(),
                      "mx.FileSearchPath()"),
            py::arg("readOptions") = (mx::XmlReadOptions*) nullptr,
            PYMATERIALX_DOCSTRING(R"docstring(
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
)docstring"));

    mod.def("writeToXmlFile", mx::writeToXmlFile,
            py::arg("doc"),
            py::arg("filename"),
            py::arg("writeOptions") = (mx::XmlWriteOptions*) nullptr,
            PYMATERIALX_DOCSTRING(R"docstring(
    Write a `Document` as XML to the given filename.

    :param doc: The `Document` to write.
    :type doc: Document
    :param filename: The filename to which data is written.
    :type filename: FilePath | str
    :param writeOptions: An optional `XmlWriteOptions` object to affect the
        behavior of the write function.
    :type writeOptions: XmlWriteOptions, optional
)docstring"));

    mod.def("writeToXmlString", mx::writeToXmlString,
            py::arg("doc"),
            py::arg("writeOptions") = nullptr,
            PYMATERIALX_DOCSTRING(R"docstring(
    Write a `Document` as XML to a string and return it.

    :param doc: The `Document` to write.
    :type doc: Document
    :param writeOptions: An optional `XmlWriteOptions` object to affect the
        behavior of the write function.
    :type writeOptions: XmlWriteOptions, optional
    :return: The output string.
)docstring"));

    mod.def("prependXInclude", mx::prependXInclude,
            py::arg("doc"),
            py::arg("filename"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Add an `XInclude` reference to the top of the given `Document`, creating a
    `GenericElement` to hold the reference filename.

    :param doc: The `Document` to be modified.
    :type doc: Document
    :param filename: The filename of the `XInclude` reference to be added.
    :type filename: FilePath
)docstring"));

    mod.def("getEnvironmentPath", &mx::getEnvironmentPath,
            py::arg_v("sep",
                      mx::PATH_LIST_SEPARATOR,
                      "mx.PATH_LIST_SEPARATOR"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Return a `FileSearchPath` object from the search path environment variable
    `MATERIALX_SEARCH_PATH`.
)docstring"));

    mod.attr("PATH_LIST_SEPARATOR") = mx::PATH_LIST_SEPARATOR;
    mod.attr("MATERIALX_SEARCH_PATH_ENV_VAR") = mx::MATERIALX_SEARCH_PATH_ENV_VAR;

    py::register_exception<mx::ExceptionParseError>(mod, "ExceptionParseError")

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    A type of exception that is raised when a requested document cannot be
    parsed.

    :see: https://materialx.org/docs/api/class_exception_parse_error.html
)docstring");

    py::register_exception<mx::ExceptionFileMissing>(mod, "ExceptionFileMissing")

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    A type of exception that is raised when a requested file cannot be opened.

    :see: https://materialx.org/docs/api/class_exception_file_missing.html
)docstring");
}
