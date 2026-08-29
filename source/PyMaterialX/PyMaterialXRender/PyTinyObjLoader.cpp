//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/TinyObjLoader.h>
#include <utility>

namespace py = pybind11;
namespace mx = MaterialX;

std::pair<bool, mx::MeshList> load_meshes_wrapper_tuple(mx::TinyObjLoader& self,
                                                        const mx::FilePath& filePath,
                                                        bool texcoordVerticalFlip)
{
    mx::MeshList meshList;
    bool success = self.load(filePath, meshList, texcoordVerticalFlip);
    return {success, meshList};
}

void bindPyTinyObjLoader(py::module& mod)
{
    py::class_<mx::TinyObjLoader, mx::TinyObjLoaderPtr, mx::GeometryLoader>(mod, "TinyObjLoader")
        .def_static("create", &mx::TinyObjLoader::create)
        .def(py::init<>())
        .def("load", &load_meshes_wrapper_tuple);
}
