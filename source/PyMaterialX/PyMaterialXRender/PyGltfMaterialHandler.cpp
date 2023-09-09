//
// Copyright Bernard Kwok
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>
#include <MaterialXRender/GltfMaterialHandler.h>

namespace py = pybind11;
namespace mx = MaterialX;


class PyMaterialHandler : public mx::MaterialHandler
{
public:
    PyMaterialHandler() :
        mx::MaterialHandler()
    {
    }

    bool load(const mx::FilePath& filePath, mx::StringVec& log) override
    {
        PYBIND11_OVERLOAD_PURE(
            bool,
            mx::MaterialHandler,
            load,
            filePath,
            log
        );
    }

    bool save(const mx::FilePath& filePath, mx::StringVec& log) override
    {
        PYBIND11_OVERLOAD_PURE(
            bool,
            mx::MaterialHandler,
            save,
            filePath,
            log
        );
    }
};

void bindPyGltfMaterialHandler(py::module& mod)
{
    py::class_<mx::MaterialHandler, PyMaterialHandler, mx::MaterialHandlerPtr>(mod, "MaterialHandler")
        .def(py::init<>())
        .def("load", &mx::MaterialHandler::load)
        .def("save", &mx::MaterialHandler::save)
        .def("extensionsSupported", &mx::MaterialHandler::extensionsSupported)
        .def("setDefinitions", &mx::MaterialHandler::setDefinitions)
        .def("getMaterials", &mx::MaterialHandler::getMaterials)
        .def("setMaterials", &mx::MaterialHandler::setMaterials)
        .def("setGenerateAssignments", &mx::MaterialHandler::setGenerateAssignments)
        .def("getGenerateAssignments", &mx::MaterialHandler::getGenerateAssignments)
        .def("setGenerateFullDefinitions", &mx::MaterialHandler::setGenerateFullDefinitions)
        .def("getGenerateFullDefinitions", &mx::MaterialHandler::getGenerateFullDefinitions)
        ;

    py::class_<mx::GltfMaterialHandler, mx::GltfMaterialHandlerPtr, mx::MaterialHandler>(mod, "GltfMaterialHandler")
        .def_static("create", &mx::GltfMaterialHandler::create)
        .def(py::init<>())
        .def("load", &mx::GltfMaterialHandler::load)
        .def("save", &mx::GltfMaterialHandler::save)
        .def("translateShaders", &mx::GltfMaterialHandler::translateShaders)
        ;
}
