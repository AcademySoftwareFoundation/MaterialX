//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/GeometryHandler.h>

namespace py = pybind11;
namespace mx = MaterialX;

class PyGeometryLoader : public mx::GeometryLoader
{
  public:
    PyGeometryLoader() :
        mx::GeometryLoader()
    {
    }

    bool load(const mx::FilePath& filePath, mx::MeshList& meshList, bool texcoordVerticalFlip = false) override
    {
        PYBIND11_OVERLOAD_PURE(
            bool,
            mx::GeometryLoader,
            load,
            filePath,
            meshList,
            texcoordVerticalFlip
        );
    }
};

void bindPyGeometryHandler(py::module& mod)
{
    py::class_<mx::GeometryLoader, PyGeometryLoader, mx::GeometryLoaderPtr>(mod, "GeometryLoader")
        .def(py::init<>())
        .def("supportedExtensions", &mx::GeometryLoader::supportedExtensions)
        .def("load", &mx::GeometryLoader::load);
    mod.attr("GeometryLoader").doc() = R"docstring(
    Base class representing a geometry loader.

    A loader can be associated with one or more file extensions.

    :see: https://materialx.org/docs/api/class_geometry_loader.html)docstring";

    py::class_<mx::GeometryHandler, mx::GeometryHandlerPtr>(mod, "GeometryHandler")
        .def(py::init<>())
        .def_static("create", &mx::GeometryHandler::create)
        .def("addLoader", &mx::GeometryHandler::addLoader)
        .def("clearGeometry", &mx::GeometryHandler::clearGeometry)
        .def("hasGeometry", &mx::GeometryHandler::hasGeometry)
        .def("getGeometry", &mx::GeometryHandler::getGeometry)
        .def("loadGeometry", &mx::GeometryHandler::loadGeometry)
        .def("getMeshes", &mx::GeometryHandler::getMeshes)
        .def("findParentMesh", &mx::GeometryHandler::findParentMesh)
        .def("getMinimumBounds", &mx::GeometryHandler::getMinimumBounds)
        .def("getMaximumBounds", &mx::GeometryHandler::getMaximumBounds);
    mod.attr("GeometryHandler").doc() = R"docstring(
    Class which holds a set of geometry loaders.

    Each loader is associated with a given set of file extensions.

    :see: https://materialx.org/docs/api/class_geometry_handler.html)docstring";
}
