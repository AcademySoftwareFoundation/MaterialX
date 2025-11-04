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
    py::class_<mx::GeometryLoader, PyGeometryLoader, mx::GeometryLoaderPtr>(mod, "GeometryLoader", "Base class representing a geometry loader.\n\nA loader can be associated with one or more file extensions.")
        .def(py::init<>())
        .def("supportedExtensions", &mx::GeometryLoader::supportedExtensions, "Get a list of extensions supported by the handler.")
        .def("load", &mx::GeometryLoader::load, "Load geometry from file path.");

    py::class_<mx::GeometryHandler, mx::GeometryHandlerPtr>(mod, "GeometryHandler", "Class which holds a set of geometry loaders.\n\nEach loader is associated with a given set of file extensions.")
        .def(py::init<>())
        .def_static("create", &mx::GeometryHandler::create)
        .def("addLoader", &mx::GeometryHandler::addLoader, "Add a geometry loader.\n\nArgs:\n    loader: Loader to add to list of available loaders.")
        .def("clearGeometry", &mx::GeometryHandler::clearGeometry, "Clear all loaded geometry.")
        .def("hasGeometry", &mx::GeometryHandler::hasGeometry)
        .def("getGeometry", &mx::GeometryHandler::getGeometry)
        .def("loadGeometry", &mx::GeometryHandler::loadGeometry, "Load geometry from a given location.\n\nArgs:\n    filePath: Path to geometry\n    texcoordVerticalFlip: Flip texture coordinates in V. Default is to not flip.")
        .def("getMeshes", &mx::GeometryHandler::getMeshes, "Get list of meshes.")
        .def("findParentMesh", &mx::GeometryHandler::findParentMesh, "Return the first mesh in our list containing the given partition.\n\nIf no matching mesh is found, then nullptr is returned.")
        .def("getMinimumBounds", &mx::GeometryHandler::getMinimumBounds, "Return the minimum bounds for all meshes.")
        .def("getMaximumBounds", &mx::GeometryHandler::getMaximumBounds, "Return the minimum bounds for all meshes.");
}
