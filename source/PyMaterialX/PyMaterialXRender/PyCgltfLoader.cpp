//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>
#include <MaterialXRender/CgltfLoader.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyCgltfLoader(py::module& mod)
{
    py::class_<mx::CgltfLoader, mx::CgltfLoaderPtr, mx::GeometryLoader>(mod, "CgltfLoader")

        .def_static("create", &mx::CgltfLoader::create,
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create a new cgltf loader.
)docstring"))

        .def(py::init<>(),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class.
)docstring"))

        .def("load", &mx::CgltfLoader::load,
             py::arg("filePath"),
             py::arg("meshFlip"),
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
    Wrapper class for a geometry loader to read glTF files using the cgltf
    library.

    Supports the following file extensions:

    * `.glb`
    * `.GLB`
    * `.gltf`
    * `.GLTF`

    :see: https://materialx.org/docs/api/class_cgltf_loader.html
    :see: https://github.com/jkuhlmann/cgltf
    :see: https://www.khronos.org/gltf/
)docstring");
}
