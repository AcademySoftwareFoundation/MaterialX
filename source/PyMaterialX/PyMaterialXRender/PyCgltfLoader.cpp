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
    py::class_<mx::CgltfLoader, mx::CgltfLoaderPtr, mx::GeometryLoader>(mod, "CgltfLoader", "Wrapper for loader to read in GLTF files using the Cgltf library.")
        .def_static("create", &mx::CgltfLoader::create, "Create a new loader.")
        .def(py::init<>())
        .def("load", &mx::CgltfLoader::load, "Load geometry from file path.");
}
