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
        .def_static("create", &mx::CgltfLoader::create)
        .def(py::init<>())
        .def("load", &mx::CgltfLoader::load)
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
