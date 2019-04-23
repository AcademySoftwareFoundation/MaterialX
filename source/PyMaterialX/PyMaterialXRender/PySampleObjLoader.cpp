//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXContrib/SampleObjLoader.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPySampleObjLoader(py::module& mod)
{
    py::class_<mx::SampleObjLoader, mx::SampleObjLoaderPtr, mx::GeometryLoader>(mod, "SampleObjLoader")
        .def_static("create", &mx::SampleObjLoader::create)
        .def(py::init<>())
        .def("load", &mx::SampleObjLoader::load)
        .def("setReadGroups", &mx::SampleObjLoader::setReadGroups)
        .def("readGroups", &mx::SampleObjLoader::readGroups);
}
