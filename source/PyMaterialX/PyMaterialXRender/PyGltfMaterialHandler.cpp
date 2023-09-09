/* 

Copyright 2022 - 2023 Bernard Kwok

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include <PyMaterialX/PyMaterialX.h>
#include <MaterialXRender/External/glTF_MaterialX/GltfMaterialHandler.h>

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
