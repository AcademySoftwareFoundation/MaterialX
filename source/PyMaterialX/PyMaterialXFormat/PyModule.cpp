//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

namespace py = pybind11;

void bindPyFile(py::module& mod);
void bindPyXmlIo(py::module& mod);
void bindPyUtil(py::module& mod);

PYBIND11_MODULE(PyMaterialXFormat, mod)
{
    mod.doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Cross-platform support for file and search paths, and XML serialization.

    All functions and classes that are defined in this module are available in
    the top-level `MaterialX` Python package, and are typically used via an
    `import` alias named `mx`:

    .. code:: python

        import MaterialX as mx

    File and Search Paths
    ---------------------

    .. autofunction:: flattenFilenames
    .. autofunction:: getEnvironmentPath
    .. autofunction:: getSourceSearchPath
    .. autofunction:: getSubdirectories
    .. autofunction:: loadDocuments
    .. autofunction:: loadLibraries
    .. autofunction:: loadLibrary
    .. autofunction:: prependXInclude

    **Classes and Enumerations**

    .. autosummary::
        :toctree: file-and-search-paths

        FilePath
        FileSearchPath
        Format
        Type

    XML Serialization
    -----------------

    .. autofunction:: readFile
    .. autofunction:: readFromXmlFile
    .. autofunction:: readFromXmlString
    .. autofunction:: writeToXmlFile
    .. autofunction:: writeToXmlString

    **Classes and Exceptions**

    .. autosummary::
        :toctree: xml-serialization

        XmlReadOptions
        XmlWriteOptions
        ExceptionParseError
        ExceptionFileMissing
)docstring");

    // PyMaterialXFormat depends on types defined in PyMaterialXCore
    PYMATERIALX_IMPORT_MODULE(PyMaterialXCore);

    bindPyFile(mod);
    bindPyXmlIo(mod);
    bindPyUtil(mod);
}
