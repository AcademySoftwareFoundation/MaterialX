//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
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
        .def("load", &mx::TinyObjLoader::load);
}
