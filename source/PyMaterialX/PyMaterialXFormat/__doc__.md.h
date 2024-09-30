//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

// Docstring for the PyMaterialXFormat module

#define PyMaterialXFormat_DOCSTRING PYMATERIALX_DOCSTRING(R"docstring(
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
