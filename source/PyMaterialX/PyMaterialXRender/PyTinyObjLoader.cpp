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

        .def_static("create", &mx::TinyObjLoader::create,
             PYMATERIALX_DOCSTRING(R"docstring(
    Create a new TinyObj loader.
)docstring"))

        .def(py::init<>(),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class.
)docstring"))

        .def("load", &mx::TinyObjLoader::load,
             py::arg("filePath"),
             py::arg("meshList"),
             py::arg("texcoordVerticalFlip") = false,
             PYMATERIALX_DOCSTRING(R"docstring(
    Load geometry from disk.

    :param filePath: Path to file to load.
    :type filePath: FilePath
    :param meshList: List of meshes to update.
    :type meshList: List[Mesh]
    :param texcoordVerticalFlip: Flip texture coordinates in V when loading.
    :type texcoordVerticalFlip: bool
    :returns: `True` if load was successful.
)docstring"))

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
