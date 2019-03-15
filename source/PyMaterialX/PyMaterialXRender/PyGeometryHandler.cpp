//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/Handlers/GeometryHandler.h>

namespace py = pybind11;
namespace mx = MaterialX;

class PyGeometryLoader : public mx::GeometryLoader
{
  public:
    PyGeometryLoader() :
        mx::GeometryLoader()
    {
    }

    bool load(const mx::FilePath& filePath, mx::MeshList& meshList) override
    {
        PYBIND11_OVERLOAD_PURE(
            bool,
            mx::GeometryLoader,
            load,
            filePath,
            meshList
        );
    }
};

void bindPyGeometryHandler(py::module& mod)
{
    py::class_<mx::GeometryLoader, PyGeometryLoader, mx::GeometryLoaderPtr>(mod, "GeometryLoader")
        .def(py::init<>())
        .def("supportedExtensions", &mx::GeometryLoader::supportedExtensions)
        .def("load", &mx::GeometryLoader::load);

    py::class_<mx::GeometryHandler, mx::GeometryHandlerPtr>(mod, "GeometryHandler")
        .def(py::init<>())
        .def("addLoader", &mx::GeometryHandler::addLoader)
        .def("clearGeometry", static_cast<void (mx::GeometryHandler::*)()>(&mx::GeometryHandler::clearGeometry))
        .def("hasGeometry", &mx::GeometryHandler::hasGeometry)
        .def("getGeometry", &mx::GeometryHandler::getGeometry)
        .def("clearGeometry", static_cast<void (mx::GeometryHandler::*)(const std::string&)>(&mx::GeometryHandler::clearGeometry))
        .def("loadGeometry", &mx::GeometryHandler::loadGeometry)
        .def("getMeshes", &mx::GeometryHandler::getMeshes)
        .def("getMinimumBounds", &mx::GeometryHandler::getMinimumBounds)
        .def("getMaximumBounds", &mx::GeometryHandler::getMaximumBounds);
}
