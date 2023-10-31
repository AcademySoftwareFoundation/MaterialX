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

        .def(py::init<>(),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class.
)docstring"))

        .def("supportedExtensions", &mx::GeometryLoader::supportedExtensions,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a set of extensions supported by the loader.
)docstring"))

        .def("load", &mx::GeometryLoader::load,
             py::arg("filePath"),
             py::arg("meshList"),
             py::arg("texcoordVerticalFlip") = false,
             PYMATERIALX_DOCSTRING(R"docstring(
    Load geometry from disk.

    This method must be implemented by derived classes.

    :param filePath: Path to file to load.
    :type filePath: FilePath
    :param meshList: List of meshes to update.
    :type meshList: List[Mesh]
    :param texcoordVerticalFlip: Flip texture coordinates in V when loading.
    :type texcoordVerticalFlip: bool
    :returns: `True` if load was successful.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Base class representing a geometry loader.

    A loader can be associated with one or more file extensions.

    :see: https://materialx.org/docs/api/class_geometry_loader.html
)docstring");

    py::class_<mx::GeometryHandler, mx::GeometryHandlerPtr>(mod, "GeometryHandler")

        .def(py::init<>(),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class.
)docstring"))

        .def_static("create", &mx::GeometryHandler::create,
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create an instance of this class.
)docstring"))

        .def("addLoader", &mx::GeometryHandler::addLoader,
             py::arg("loader"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a geometry loader.

    :param loader: Loader to add to list of available loaders.
    :type loader: GeometryLoader
)docstring"))

        .def("clearGeometry", &mx::GeometryHandler::clearGeometry,
             PYMATERIALX_DOCSTRING(R"docstring(
    Clear all loaded geometry.
)docstring"))

        .def("hasGeometry", &mx::GeometryHandler::hasGeometry,
             py::arg("location"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Determine if any meshes have been loaded from a given `location`.
)docstring"))

        .def("getGeometry", &mx::GeometryHandler::getGeometry,
             py::arg("meshes"),
             py::arg("location"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Find all meshes loaded from a given `location`.
)docstring"))

        .def("loadGeometry", &mx::GeometryHandler::loadGeometry,
             py::arg("filePath"),
             py::arg("texcoordVerticalFlip") = false,
             PYMATERIALX_DOCSTRING(R"docstring(
    Load geometry from a given location.

    :param filePath: Path to geometry.
    :type filePath: FilePath
    :param texcoordVerticalFlip: Flip texture coordinates in V. Default is to
        not flip.
    :type texcoordVerticalFlip: bool
)docstring"))

        .def("getMeshes", &mx::GeometryHandler::getMeshes,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return list of meshes.
)docstring"))

        .def("findParentMesh", &mx::GeometryHandler::findParentMesh,
             py::arg("part"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the first mesh in our list containing the given partition.

    If no matching mesh is found, then `None` is returned.
)docstring"))

        .def("getMinimumBounds", &mx::GeometryHandler::getMinimumBounds,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the minimum bounds for all meshes.
)docstring"))

        .def("getMaximumBounds", &mx::GeometryHandler::getMaximumBounds,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the maximum bounds for all meshes.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class which holds a set of geometry loaders.

    Each loader is associated with a given set of file extensions.

    :see: https://materialx.org/docs/api/class_geometry_handler.html
)docstring");
}
