//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Node.h>
#include <MaterialXCore/Util.h>

#include <MaterialXFormat/File.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyUtil(py::module& mod)
{
    mod.def("getVersionString", &mx::getVersionString,
            PYMATERIALX_DOCSTRING(R"docstring(
    Return the version of the MaterialX library as a string.

    >>> import MaterialX as mx
    >>> mx.getVersionString()
    '1.38.9'

    The resulting version is also available in the `__version__` attribute of
    the `MaterialX` Python package:

    >>> import MaterialX
    >>> MaterialX.__version__
    '1.38.9'
)docstring"));

    mod.def("getVersionIntegers", &mx::getVersionIntegers,
            PYMATERIALX_DOCSTRING(R"docstring(
    Return the major, minor, and build versions of the MaterialX library as an
    integer tuple.

    >>> import MaterialX as mx
    >>> mx.getVersionIntegers()
    (1, 38, 9)
)docstring"));

    mod.def("createValidName", &mx::createValidName,
            py::arg("name"), py::arg("replaceChar") = '_',
            PYMATERIALX_DOCSTRING(R"docstring(
    Create a valid MaterialX name from the given `name`,
    replacing invalid characters with the given `replaceChar`.

    >>> import MaterialX as mx
    >>> mx.createValidName('left arm #123')
    'left_arm__123'
)docstring"));

    mod.def("isValidName", &mx::isValidName,
            py::arg("name"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if the given `name` is a valid MaterialX name,
    otherwise False.

    >>> import MaterialX as mx
    >>> mx.isValidName('left arm #123')
    False
)docstring"));

    mod.def("incrementName", &mx::incrementName,
            py::arg("name"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Increment the numeric suffix of the given `name`.

    >>> import MaterialX as mx
    >>> mx.incrementName('left arm #123')
    'left arm #124'
)docstring"));

    mod.def("splitString", &mx::splitString,
            py::arg("string"), py::arg("sep"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Split the given `string` into a list of substrings using the
    given set of separator characters.

    >>> import MaterialX as mx
    >>> mx.splitString('MaterialX', 'aeiou')
    ['M', 't', 'r', 'lX']
)docstring"));

    mod.def("joinStrings", &mx::joinStrings,
            py::arg("strings"), py::arg("sep"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Join a list of substrings into a single string, placing the
    given separator between each substring.

    >>> import MaterialX as mx
    >>> mx.joinStrings(['M', 'teri', 'lX'], 'a')
    'MaterialX'
)docstring"));

    mod.def("replaceSubstrings", &mx::replaceSubstrings,
            py::arg("string"), py::arg("substitutions"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Apply the given substring `substitutions` to the input `string`.

    >>> import MaterialX as mx
    >>> mx.replaceSubstrings(
    ...     '/bob/alice/guido',
    ...     {'bob': 'b', 'alice': 'a', 'guido': 'g'}
    ... )
    '/b/a/g'
)docstring"));

    mod.def("stringStartsWith", &mx::stringStartsWith,
            py::arg("string"), py::arg("prefix"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if the given `string` starts with the given `prefix`,
    otherwise `False`.

    >>> import MaterialX as mx
    >>> mx.stringStartsWith('turbulence3d', 'turbulence')
    True
    >>> mx.stringStartsWith('turbulence3d', 'Turbulence')
    False
)docstring"));

    mod.def("stringEndsWith", &mx::stringEndsWith,
            py::arg("string"), py::arg("suffix"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if the given `string` ends with the given `suffix`,
    otherwise `False`.

    >>> import MaterialX as mx
    >>> mx.stringEndsWith('turbulence3d', '3d')
    True
    >>> mx.stringEndsWith('turbulence3d', '3D')
    False
)docstring"));

    mod.def("splitNamePath", &mx::splitNamePath,
            py::arg("namePath"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Split the given `namePath` into a list of names.

    >>> import MaterialX as mx
    >>> mx.splitNamePath('/robot2/right_arm')
    ['robot2', 'right_arm']
)docstring"));

    mod.def("createNamePath", &mx::createNamePath,
            py::arg("names"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Create a name path from the given list of `names`.

    >>> import MaterialX as mx
    >>> mx.createNamePath(['robot2', 'right_arm'])
    'robot2/right_arm'
)docstring"));

    mod.def("parentNamePath", &mx::parentNamePath,
            py::arg("namePath"),
            PYMATERIALX_DOCSTRING(R"docstring(
    Given a `namePath`, return the parent name path.

    >>> import MaterialX as mx
    >>> mx.parentNamePath('/robot2/right_arm')
    'robot2'
)docstring"));
}
