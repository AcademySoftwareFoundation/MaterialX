//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/TinyObjLoader.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyTinyObjLoader(py::module& mod)
{
    py::class_<mx::TinyObjLoader, mx::TinyObjLoaderPtr, mx::GeometryLoader>(mod, "TinyObjLoader")
        .def_static("create", &mx::TinyObjLoader::create)
        .def(py::init<>())
        .def("load", &mx::TinyObjLoader::load)
        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Wrapper class for a geometry loader to read OBJ files using the TinyObj
    library.

    Supports the following file extensions:

    * `.obj`
    * `.OBJ`

    :see: https://materialx.org/docs/api/class_tiny_obj_loader.html
    :see: https://github.com/tinyobjloader/tinyobjloader/
)docstring");
}
